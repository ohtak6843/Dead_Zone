#include "pch.h"
#include "LoadingScene.h"
#include "SceneMgr.h"
#include "Resources.h"
#include "UICamera.h"
#include "Camera.h"
#include "Transform.h"
#include "MeshRenderer.h"
#include "Material.h"
#include "Texture.h"

#include "TitleScene.h"
#include "Stage01.h"

LoadingScene::LoadingScene()
{
}

LoadingScene::~LoadingScene()
{
}

void LoadingScene::Init()
{
#pragma region Camera
	{
		shared_ptr<UICamera> camera = make_shared<UICamera>();
		camera->SetName(L"UICamera");
		camera->SetTransform(make_shared<Transform>());
		camera->SetCamera(make_shared<Camera>());
		camera->GetTransform()->SetLocalPosition(Vec3(0.f, 0.f, 0.f));
		camera->GetCamera()->SetProjectionType(PROJECTION_TYPE::ORTHOGRAPHIC);
		uint8 layerIndex = GET_SINGLE(SceneMgr)->LayerNameToIndex(L"UI");
		camera->GetCamera()->SetCullingMaskAll();
		camera->GetCamera()->SetCullingMaskLayerOnOff(layerIndex, false); // UI
		AddGameObject(camera);
	}
#pragma endregion

#pragma region LoadingImage
	{
		shared_ptr<GameObject> loadingImage = make_shared<GameObject>();
		loadingImage->SetLayerIndex(GET_SINGLE(SceneMgr)->LayerNameToIndex(L"UI"));
		loadingImage->SetTransform(make_shared<Transform>());
		loadingImage->GetTransform()->SetLocalScale(Vec3(1280.f, 800.f, 1.f));
		loadingImage->GetTransform()->SetLocalPosition(Vec3(0.f, 0.f, 1.f));
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		{
			shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
			meshRenderer->SetMesh(mesh);
		}
		{
			shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"AlphaTexture");
			shared_ptr<Texture> texture = GET_SINGLE(Resources)->Load<Texture>(L"LoadingImage", L"..\\Resources\\Texture\\LoadingImage.png");
			shared_ptr<Material> material = make_shared<Material>();
			material->SetShader(shader);
			material->SetTexture(0, texture);
			meshRenderer->SetMaterial(material);
		}

		loadingImage->SetMeshRenderer(meshRenderer);
		AddGameObject(loadingImage);
	}
#pragma endregion
}