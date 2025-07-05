#include "pch.h"
#include "TitleScene.h"
#include "Transform.h"
#include "MeshRenderer.h"
#include "Mesh.h"
#include "Camera.h"
#include "TitleCamera.h"

#include "InputMgr.h"
#include "SceneMgr.h"
#include "Resources.h"

#include "Framework.h"
#include "BaseCollider.h"
#include "OrientedBoxCollider.h"

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
		shared_ptr<TitleCamera> camera = make_shared<TitleCamera>();
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

#pragma region TitleImage
	{
		shared_ptr<GameObject> titleImage = make_shared<GameObject>();
		titleImage->SetLayerIndex(GET_SINGLE(SceneMgr)->LayerNameToIndex(L"UI"));
		titleImage->SetTransform(make_shared<Transform>());
		titleImage->GetTransform()->SetLocalScale(Vec3(1280.f, 800.f, 1.f));
		titleImage->GetTransform()->SetLocalPosition(Vec3(0.f, 0.f, 1.f));
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		{
			shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
			meshRenderer->SetMesh(mesh);
		}
		{
			shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"AlphaTexture");
			shared_ptr<Texture> texture = GET_SINGLE(Resources)->Load<Texture>(L"TitleImage", L"..\\Resources\\Texture\\TitleScene\\TitleImage.png");
			shared_ptr<Material> material = make_shared<Material>();
			material->SetShader(shader);
			material->SetTexture(0, texture);
			meshRenderer->SetMaterial(material);
		}

		titleImage->SetMeshRenderer(meshRenderer);
		AddGameObject(titleImage);
	}
#pragma endregion

#pragma region 
	{
		shared_ptr<GameObject> gameStartButton = make_shared<GameObject>();
		gameStartButton->SetName(L"GameStartButton");
		gameStartButton->SetLayerIndex(GET_SINGLE(SceneMgr)->LayerNameToIndex(L"UI"));
		gameStartButton->SetTransform(make_shared<Transform>());
		gameStartButton->GetTransform()->SetLocalScale(Vec3(400.f, 200.f, 1.f));
		gameStartButton->GetTransform()->SetLocalPosition(Vec3(380.f, -280.f, 1.f));
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		{
			shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
			meshRenderer->SetMesh(mesh);
		}
		{
			shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"AlphaTexture");
			shared_ptr<Texture> texture = GET_SINGLE(Resources)->Load<Texture>(L"GameStartButton", L"..\\Resources\\Texture\\TitleScene\\GameStartButton.png");
			shared_ptr<Material> material = make_shared<Material>();
			material->SetShader(shader);
			material->SetTexture(0, texture);
			meshRenderer->SetMaterial(material);
		}
		gameStartButton->SetMeshRenderer(meshRenderer);

		//shared_ptr<OrientedBoxCollider> boundingBox = make_shared<OrientedBoxCollider>();
		//boundingBox->SetCenter(Vec3(0.f, 0.f, 0.f));
		//boundingBox->SetExtents(Vec3(0.5f, 0.5f, 0.f));
		//gameStartButton->SetCollider(boundingBox);
		gameStartButton->SetCheckFrustum(false);
		AddGameObject(gameStartButton);
	}
#pragma endregion

	INPUT->LockCursor(false);
	gameFramework->ToggleFullScreen(false);
}