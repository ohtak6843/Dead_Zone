#include "pch.h"
#include "RenderPass.h"
#include "Framework.h"
#include "GameObject.h"
#include "Camera.h"
#include "Light.h"
#include "Scene.h"
#include "SceneMgr.h"
#include "MeshRenderer.h"
#include "Material.h"
#include "InstancingMgr.h"
#include "Transform.h"
#include "Resources.h"
#include "Particle.h"

#include "BaseCollider.h"
#include "SphereCollider.h"
#include "OrientedBoxCollider.h"

#include "CameraObject.h"
#include "ParticleObject.h"

RenderPass::RenderPass()
{
}

RenderPass::~RenderPass()
{
	// 모든 카메라와 라이트 제거
	_cameras.clear();
	_lights.clear();
}

void RenderPass::Render()
{
	// 버퍼 초기화
	ClearBuffers();
	ClearRenderTargetView();

	// 렌더링
	UploadLightData();
	RenderShadow();
	RenderDeferred();
	RenderLights();
	RenderFinal();
	RenderForwardandParticle();
}

void RenderPass::RemoveCamera(shared_ptr<class Camera> camera)
{
	auto findIt = std::find(_cameras.begin(), _cameras.end(), camera);
	if (findIt != _cameras.end())
		_cameras.erase(findIt);
}

void RenderPass::RemoveLight(shared_ptr<class Light> light)
{
	auto findIt = std::find(_lights.begin(), _lights.end(), light);
	if (findIt != _lights.end())
		_lights.erase(findIt);
}

shared_ptr<class Camera> RenderPass::GetMainCamera()
{
	if (_cameras.empty())
		return nullptr;

	return _cameras[0];
}

shared_ptr<Camera> RenderPass::GetPlayerCamera()
{
	for (const shared_ptr<Camera>& camera : _cameras)
	{
		if (camera->GetGameObject()->GetName() == L"Player_Camera")
			return camera;
	}

	return nullptr;
}

shared_ptr<class Camera> RenderPass::GetGunCamera()
{
	for (const shared_ptr<Camera>& camera : _cameras)
	{
		if (camera->GetGameObject()->GetName() == L"Gun_Camera")
			return camera;
	}

	return nullptr;
}

void RenderPass::ClearBuffers()
{
	_vecDeferred.clear();
	_vecForward.clear();
	_vecParticle.clear();
	_vecShadow.clear();
}

void RenderPass::SortGameObjects(shared_ptr<Camera> camera)
{
	shared_ptr<Scene> scene = GET_SINGLE(SceneMgr)->GetActiveScene();
	const vector<shared_ptr<GameObject>>& gameObjects = scene->GetGameObjects();

	_vecForward.clear();
	_vecDeferred.clear();
	_vecParticle.clear();

	for (auto& gameObject : gameObjects)
	{
		if (gameObject->GetMeshRenderer() == nullptr && gameObject->GetGameObjectType() != GAMEOBJECT_TYPE::PARTICLE)
			continue;

		if (camera->IsCulled(gameObject->GetLayerIndex()))
			continue;

		if (gameObject->GetCheckFrustum())
		{
			if (camera->FrustumCulling(gameObject) == DISJOINT)
			{
				continue;
			}
		}

		if (gameObject->GetMeshRenderer())
		{
			SHADER_TYPE shaderType = gameObject->GetMeshRenderer()->GetMaterial()->GetShader()->GetShaderType();
			switch (shaderType)
			{
			case SHADER_TYPE::DEFERRED:
				_vecDeferred.push_back(gameObject);
				{
					shared_ptr<BaseCollider> collider = gameObject->GetCollider();
					if (collider && DEBUG_MODE)
					{
						_vecDeferred.push_back(collider->GetDebugCollider());
					}
				}
				break;
			case SHADER_TYPE::FORWARD:
				_vecForward.push_back(gameObject);
				break;
			}
		}
		else
		{
			_vecParticle.push_back(static_pointer_cast<ParticleObject>(gameObject));
		}
	}
}

void RenderPass::SortShadowGameObjects(shared_ptr<Camera> camera)
{
	shared_ptr<Scene> scene = GET_SINGLE(SceneMgr)->GetActiveScene();
	const vector<shared_ptr<GameObject>>& gameObjects = scene->GetGameObjects();

	_vecShadow.clear();

	for (auto& gameObject : gameObjects)
	{
		if (gameObject->GetMeshRenderer() == nullptr)
			continue;

		if (gameObject->IsStatic())
			continue;

		if (camera->IsCulled(gameObject->GetLayerIndex()))
			continue;

		if (gameObject->GetCheckFrustum())
		{
			if (camera->FrustumCulling(gameObject) == DISJOINT)
			{
				continue;
			}
		}

		_vecShadow.push_back(gameObject);
	}
}

void RenderPass::UploadLightData()
{
	LightParams lightParams = {};

	for (auto& light : _lights)
	{
		const LightInfo& lightInfo = light->GetLightInfo();

		light->SetLightIndex(lightParams.lightCount);

		lightParams.lights[lightParams.lightCount] = lightInfo;
		lightParams.lightCount++;
	}

	CONST_BUFFER(CONSTANT_BUFFER_TYPE::LIGHT)->SetGraphicsGlobalData(&lightParams, sizeof(lightParams));
}

void RenderPass::ClearRenderTargetView()
{
	// SwapChain Back Buffer 초기화
	int8 backIndex = gameFramework->GetCurrBackBufferIndex();
	gameFramework->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)->ClearRenderTargetView(backIndex);

	gameFramework->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SHADOW)->ClearRenderTargetView();
	gameFramework->GetRTGroup(RENDER_TARGET_GROUP_TYPE::G_BUFFER)->ClearRenderTargetView();
	gameFramework->GetRTGroup(RENDER_TARGET_GROUP_TYPE::LIGHTING)->ClearRenderTargetView();
}

void RenderPass::RenderShadow()
{
	gameFramework->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SHADOW)->OMSetRenderTargets();

	for (auto& light : _lights)
	{
		if (light->GetLightType() != LIGHT_TYPE::DIRECTIONAL_LIGHT)
			continue;

		shared_ptr<Camera> camera = light->GetShadowCamera()->GetCamera();
		SortShadowGameObjects(camera);
		Camera::S_MatView = camera->GetViewMatrix();
		Camera::S_MatProjection = camera->GetProjectionMatrix();

		for (auto& shadowObject : _vecShadow)
		{
			shadowObject->GetMeshRenderer()->RenderShadow();
		}
	}

	gameFramework->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SHADOW)->WaitTargetToResource();
}

void RenderPass::RenderDeferred()
{
	gameFramework->GetRTGroup(RENDER_TARGET_GROUP_TYPE::G_BUFFER)->OMSetRenderTargets();

	shared_ptr<Camera> camera = GetMainCamera();
	SortGameObjects(camera);
	Camera::S_MatView = camera->GetViewMatrix();
	Camera::S_MatProjection = camera->GetProjectionMatrix();

	GET_SINGLE(InstancingMgr)->Render(_vecDeferred);

	gameFramework->GetRTGroup(RENDER_TARGET_GROUP_TYPE::G_BUFFER)->WaitTargetToResource();
}

void RenderPass::RenderLights()
{
	gameFramework->GetRTGroup(RENDER_TARGET_GROUP_TYPE::LIGHTING)->OMSetRenderTargets();

	shared_ptr<Camera> camera = GetMainCamera();
	Camera::S_MatView = camera->GetViewMatrix();
	Camera::S_MatProjection = camera->GetProjectionMatrix();

	for (auto& light : _lights)
	{
		int8 lightIndex = light->GetLightIndex();
		LightInfo lightInfo = light->GetLightInfo();
		assert(lightIndex >= 0);

		light->GetTransform()->PushData();

		if (static_cast<LIGHT_TYPE>(lightInfo.lightType) == LIGHT_TYPE::DIRECTIONAL_LIGHT)
		{
			shared_ptr<Texture> shadowTex = GET_SINGLE(Resources)->Get<Texture>(L"ShadowTarget");
			light->GetLightMaterial()->SetTexture(2, shadowTex);

			Matrix matVP = light->GetShadowCamera()->GetCamera()->GetViewMatrix() * light->GetShadowCamera()->GetCamera()->GetProjectionMatrix();
			light->GetLightMaterial()->SetMatrix(0, matVP);
		}
		else
		{
			float scale = 2 * lightInfo.range;
			light->GetTransform()->SetLocalScale(Vec3(scale, scale, scale));
		}

		light->GetLightMaterial()->SetInt(0, lightIndex);
		light->GetLightMaterial()->PushGraphicsData();

		light->GetVolumeMesh()->Render();
	}

	gameFramework->GetRTGroup(RENDER_TARGET_GROUP_TYPE::LIGHTING)->WaitTargetToResource();
}

void RenderPass::RenderFinal()
{
	int8 backIndex = gameFramework->GetCurrBackBufferIndex();
	gameFramework->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)->OMSetRenderTargets(1, backIndex);

	GET_SINGLE(Resources)->Get<Material>(L"Final")->PushGraphicsData();
	GET_SINGLE(Resources)->Get<Mesh>(L"Rectangle")->Render();
}

void RenderPass::RenderForwardandParticle()
{
	shared_ptr<Camera> mainCamera = GetMainCamera();

	Camera::S_MatView = mainCamera->GetViewMatrix();
	Camera::S_MatProjection = mainCamera->GetProjectionMatrix();

	GET_SINGLE(InstancingMgr)->Render(_vecForward);

	for (auto& particle : _vecParticle)
	{
		particle->GetParticle()->Render();
	}

	for (auto& camera : _cameras)
	{
		if (camera == mainCamera)
			continue;

		SortGameObjects(camera);

		Camera::S_MatView = camera->GetViewMatrix();
		Camera::S_MatProjection = camera->GetProjectionMatrix();

		GET_SINGLE(InstancingMgr)->Render(_vecForward);

		for (auto& particle : _vecParticle)
		{
			particle->GetParticle()->Render();
		}
	}
}
