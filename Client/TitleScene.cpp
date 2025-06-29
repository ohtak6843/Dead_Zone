#include "pch.h"
#include "TitleScene.h"
#include "Transform.h"
#include "MeshRenderer.h"
#include "Mesh.h"
#include "Camera.h"
#include "UICamera.h"

#include "SceneMgr.h"
#include "Resources.h"

TitleScene::TitleScene()
{
}

TitleScene::~TitleScene()
{
}

void TitleScene::Init()
{
#pragma region Camera
	{
		shared_ptr<UICamera> camera = make_shared<UICamera>();
		camera->SetName(L"TitleCamera");
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
			shared_ptr<Texture> texture = GET_SINGLE(Resources)->Load<Texture>(L"Loading", L"..\\Resources\\Texture\\LoadingImage.jpg");
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