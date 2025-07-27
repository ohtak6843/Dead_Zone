#include "pch.h"
#include "SceneMgr.h"
#include "Scene.h"

#include "Framework.h"
#include "Material.h"
#include "GameObject.h"
#include "MeshRenderer.h"
#include "Transform.h"
#include "Camera.h"
#include "Light.h"

#include "LocalPlayer.h"
#include "Resources.h"
#include "Particle.h"
#include "BaseCollider.h"
#include "SphereCollider.h"
#include "OrientedBoxCollider.h"
#include "MeshData.h"

#include "CameraObject.h"
#include "UICamera.h"
#include "PlayerCamera.h"
#include "GunCamera.h"

#include "LightObject.h"

#include "Zombie.h"
#include "M4A1.h"
#include "AK47.h"

#include "Scene.h"
#include "LoadingScene.h"
#include "TitleScene.h"
#include "Stage01.h"
#include "Stage02.h"

#include "UIMgr.h"
#include "InputMgr.h"

// TODO: 나중에 삭제
#include "Timer.h"
#include <sstream>

#include "JsonMgr.h"

void SceneMgr::Update()
{
	if (_activeScene == nullptr)
		return;

	_activeScene->Update();
	_activeScene->LateUpdate();
	_activeScene->FinalUpdate();
}

void SceneMgr::Render()
{
	if (_activeScene)
		_activeScene->Render();
}

void SceneMgr::RenderUI()
{
	if (_activeScene == nullptr || GetSceneType() == SCENE_TYPE::LOADING)
		return;

	gameFramework->BeginD2DRender();

	auto device = gameFramework->GetD3D11on12Device();
	auto ctx = device->GetD2DDeviceContext();
	auto brush = device->GetSolidColorBrush();

	brush->SetColor(D2D1::ColorF(D2D1::ColorF::White)); // 텍스트 색 설정

	if (_activeScene && (_sceneType == SCENE_TYPE::STAGE01 || _sceneType == SCENE_TYPE::STAGE02))
	{
		//// 플레이어 보는 방향
		//{
		//	Vec2 pivot = { static_cast<float>(gameFramework->GetWindow().width - 100), static_cast<float>(gameFramework->GetWindow().height / 2) };
		//	D2D1_RECT_F textRect = D2D1::RectF(pivot.x - 100, pivot.y - 200, pivot.x + 100, pivot.y + 200);

		//	// LightPos
		//	std::wstringstream wss;
		//	wstring text = L"P_X : ";
		//	Vec3 pLook = _activeScene->FindGameObject(L"LocalPlayer")->GetTransform()->GetLook();
		//	wss.str(L"");
		//	wss.clear();
		//	wss << std::fixed << std::setprecision(2) << pLook.x;
		//	text += wss.str();
		//	text += L"\nP_Y : ";
		//	wss.str(L"");
		//	wss.clear();
		//	wss << std::fixed << std::setprecision(2) << pLook.y;
		//	text += wss.str();
		//	text += L"\nP_Z : ";
		//	wss.str(L"");
		//	wss.clear();
		//	wss << std::fixed << std::setprecision(2) << pLook.z;
		//	text += wss.str();

		//	device->GetD2DDeviceContext()->DrawTextW(
		//		text.c_str(),
		//		static_cast<uint32>(text.size()),
		//		device->GetTextFormat().Get(),
		//		&textRect,
		//		device->GetSolidColorBrush().Get());
		//}

		//// 플레이어 위치
		//{
		//	Vec2 pivot = { static_cast<float>(100), static_cast<float>(gameFramework->GetWindow().height / 2) };
		//	D2D1_RECT_F textRect = D2D1::RectF(pivot.x - 100, pivot.y - 200, pivot.x + 100, pivot.y + 200);

		//	// LightPos
		//	std::wstringstream wss;
		//	wstring text = L"P_X : ";
		//	Vec3 pLook = _activeScene->FindGameObject(L"LocalPlayer")->GetTransform()->GetWorldPosition();
		//	wss.str(L"");
		//	wss.clear();
		//	wss << std::fixed << std::setprecision(2) << pLook.x;
		//	text += wss.str();
		//	text += L"\nP_Y : ";
		//	wss.str(L"");
		//	wss.clear();
		//	wss << std::fixed << std::setprecision(2) << pLook.y;
		//	text += wss.str();
		//	text += L"\nP_Z : ";
		//	wss.str(L"");
		//	wss.clear();
		//	wss << std::fixed << std::setprecision(2) << pLook.z;
		//	text += wss.str();

		//	device->GetD2DDeviceContext()->DrawTextW(
		//		text.c_str(),
		//		static_cast<uint32>(text.size()),
		//		device->GetTextFormat().Get(),
		//		&textRect,
		//		device->GetSolidColorBrush().Get());
		//}
		// 잔여탄 UI
		{
			// 현재 총알 수를 가져오는 로직 이상 생김. 수정 필요
			int32 currentAmmo = 0;
			int32 gunType = static_pointer_cast<LocalPlayer>(_activeScene->FindGameObject(L"LocalPlayer"))->getGunType();
			if (gunType == 0)
				currentAmmo = static_pointer_cast<M4A1>(_activeScene->FindGameObject(L"M4A1"))->GetCurrentAmmo();
			else if (gunType == 1)
				currentAmmo = static_pointer_cast<AK47>(_activeScene->FindGameObject(L"AK47"))->GetCurrentAmmo();

			currentAmmo = 30;
			std::wstringstream wss;
			wss << currentAmmo;

			GET_SINGLE(UIMgr)->DrawTextUI(
				wss.str(),
				Vec2(995.f, 740.f),			// 위치
				Vec2(100.f, 100.f),			// 텍스트 상자 크기
				16,							// 폰트 크기 ( 8 ~ 128까지 짝수 사이즈만 설정 가능)
				D2D1::ColorF::White,		// 텍스트 색상
				D2D1::ColorF(0, 0, 0, 0.0f) // 배경 색
			);
		}


		// LocalPlayer HP UI
		{
			std::wstringstream wss;
			wstring name = static_pointer_cast<Player>(_activeScene->FindGameObject(L"LocalPlayer"))->GetName();
			uint32 hp = static_pointer_cast<Player>(_activeScene->FindGameObject(L"LocalPlayer"))->GetHp();
			wss << name << "1 - " << hp;
			GET_SINGLE(UIMgr)->DrawTextUI(
				wss.str(),
				Vec2(10.f, 740.f),
				Vec2(300.f, 100.f),
				16,
				D2D1::ColorF::White,
				D2D1::ColorF(0, 0, 0, 0.0f)
			);
			auto hpber = _activeScene->FindGameObject(L"PlayerPanel_1_HP");
			if (hpber)
			{
				uint32 maxHp = 100;
				float hpRatio = static_cast<float>(hp) / maxHp;

				const float fullWidth = 270.f; // 전체 HP바 너비
				Vec3 scale = hpber->GetTransform()->GetLocalScale();
				scale.x = fullWidth * hpRatio;
				hpber->GetTransform()->SetLocalScale(scale);


				Vec3 position = hpber->GetTransform()->GetLocalPosition();
				position.x = -495.f - (fullWidth * (1.f - hpRatio) * 0.5f);  // 왼쪽 기준 보정
				hpber->GetTransform()->SetLocalPosition(position);
			}
		}

		// TODO: 나중에 플레이어2, 3의 HP를 실제로 가져와야 함
		// Player2 HP UI
		{
			std::wstringstream wss;
			wstring name = static_pointer_cast<Player>(_activeScene->FindGameObject(L"LocalPlayer"))->GetName();
			uint32 hp = static_pointer_cast<Player>(_activeScene->FindGameObject(L"LocalPlayer"))->GetHp();
			hp = 85;
			wss << name << "2 - " << hp;
			GET_SINGLE(UIMgr)->DrawTextUI(
				wss.str(),
				Vec2(10.f, 740.f - 60.f),
				Vec2(300.f, 100.f),
				16,
				D2D1::ColorF::White,
				D2D1::ColorF(0, 0, 0, 0.0f)
			);
			auto hpber = _activeScene->FindGameObject(L"PlayerPanel_2_HP");
			if (hpber)
			{
				uint32 maxHp = 100;
				float hpRatio = static_cast<float>(hp) / maxHp;

				const float fullWidth = 270.f; // 전체 HP바 너비
				Vec3 scale = hpber->GetTransform()->GetLocalScale();
				scale.x = fullWidth * hpRatio;
				hpber->GetTransform()->SetLocalScale(scale);


				Vec3 position = hpber->GetTransform()->GetLocalPosition();
				position.x = -495.f - (fullWidth * (1.f - hpRatio) * 0.5f);  // 왼쪽 기준 보정
				hpber->GetTransform()->SetLocalPosition(position);
			}
		}

		// Player3 HP UI
		{
			std::wstringstream wss;
			wstring name = static_pointer_cast<Player>(_activeScene->FindGameObject(L"LocalPlayer"))->GetName();
			uint32 hp = static_pointer_cast<Player>(_activeScene->FindGameObject(L"LocalPlayer"))->GetHp();
			hp = 55;
			wss << name << "3 - " << hp;
			GET_SINGLE(UIMgr)->DrawTextUI(
				wss.str(),
				Vec2(10.f, 740.f - 120.f),
				Vec2(300.f, 100.f),
				16,
				D2D1::ColorF::White,
				D2D1::ColorF(0, 0, 0, 0.0f)
			);
			auto hpber = _activeScene->FindGameObject(L"PlayerPanel_3_HP");
			if (hpber)
			{
				uint32 maxHp = 100;
				float hpRatio = static_cast<float>(hp) / maxHp;

				const float fullWidth = 270.f; // 전체 HP바 너비
				Vec3 scale = hpber->GetTransform()->GetLocalScale();
				scale.x = fullWidth * hpRatio;
				hpber->GetTransform()->SetLocalScale(scale);


				Vec3 position = hpber->GetTransform()->GetLocalPosition();
				position.x = -495.f - (fullWidth * (1.f - hpRatio) * 0.5f);  // 왼쪽 기준 보정
				hpber->GetTransform()->SetLocalPosition(position);
			}
		}
	}

	gameFramework->EndD2DRender();
}

void SceneMgr::LoadScene(SCENE_TYPE type)
{
	switch (type)
	{
	case SCENE_TYPE::TITLE:
		GET_SINGLE(SceneMgr)->SetSceneType(SCENE_TYPE::TITLE);
		_activeScene = make_shared<TitleScene>();
		break;
	case SCENE_TYPE::STAGE01:
		GET_SINGLE(SceneMgr)->SetSceneType(SCENE_TYPE::STAGE01);
		_activeScene = make_shared<Stage01>();
		break;
	case SCENE_TYPE::STAGE02:
		GET_SINGLE(SceneMgr)->SetSceneType(SCENE_TYPE::STAGE02);
		_activeScene = make_shared<Stage02>();
		break;
	}

	_activeScene->LoadResources();
	_activeScene->Init();
	_activeScene->Awake();
	_activeScene->Start();
}

void SceneMgr::SwitchScene(SCENE_TYPE type)
{
	GET_SINGLE(UIMgr)->ClearUI(); // UI 초기화
	GET_SINGLE(InputMgr)->ClearState();

	GET_SINGLE(SceneMgr)->SetSceneType(SCENE_TYPE::LOADING);
	_activeScene = make_shared<LoadingScene>();
	_activeScene->LoadResources();
	_activeScene->Init();
	_activeScene->Awake();
	_activeScene->Start();
	gameFramework->Update();

	LoadScene(type);
}

void SceneMgr::SetLayerName(uint8 index, const wstring& name)
{
	// 기존 데이터 삭제
	const wstring& prevName = _layerNames[index];
	_layerIndex.erase(prevName);

	_layerNames[index] = name;
	_layerIndex[name] = index;
}

uint8 SceneMgr::LayerNameToIndex(const wstring& name)
{
	auto findIt = _layerIndex.find(name);
	if (findIt == _layerIndex.end())
		return 0;

	return findIt->second;
}

shared_ptr<Scene> SceneMgr::LoadLoadingScene()
{
#pragma region LayerMask
	SetLayerName(0, L"Default");
	SetLayerName(1, L"UI");
#pragma endregion

	shared_ptr<Scene> scene = make_shared<Scene>();

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
		scene->AddGameObject(camera);
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
		scene->AddGameObject(loadingImage);
	}	
#pragma endregion

	return scene;
}

shared_ptr<Scene> SceneMgr::LoadStage01()
{
#pragma region LayerMask
	SetLayerName(0, L"Default");
	SetLayerName(1, L"UI");
	SetLayerName(2, L"Gun"); // 총 UI 별도 처리
#pragma endregion

#pragma region ComputeShader
	{
		shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"ComputeShader");

		// UAV 용 Texture 생성
		shared_ptr<Texture> texture = GET_SINGLE(Resources)->CreateTexture(L"UAVTexture",
			DXGI_FORMAT_R8G8B8A8_UNORM, 1024, 1024,
			CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

		shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"ComputeShader");
		material->SetShader(shader);
		material->SetInt(0, 1);
		gameFramework->GetComputeDescHeap()->SetUAV(texture->GetUAVHandle(), UAV_REGISTER::u0);

		// 쓰레드 그룹 (1 * 1024 * 1)
		material->Dispatch(1, 1024, 1);
	}
#pragma endregion

	shared_ptr<Scene> scene = make_shared<Scene>();

#pragma region Camera
	{
		shared_ptr<PlayerCamera> camera = make_shared<PlayerCamera>();
		camera->SetName(L"Player_Camera");
		camera->SetTransform(make_shared<Transform>());
		camera->SetCamera(make_shared<Camera>());
		camera->GetCamera()->SetFar(10000.f);
		camera->GetCamera()->SetFOV(90.f);
		camera->GetTransform()->SetLocalPosition(Vec3(0.f, 0.f, 0.f));
		uint8 layerIndex = GET_SINGLE(SceneMgr)->LayerNameToIndex(L"UI");
		camera->GetCamera()->SetCullingMaskLayerOnOff(layerIndex, true); // UI는 안 찍음

		layerIndex = GET_SINGLE(SceneMgr)->LayerNameToIndex(L"Gun");
		camera->GetCamera()->SetCullingMaskLayerOnOff(layerIndex, true); // Gun은 안 찍음

		scene->AddGameObject(camera);
	}
#pragma endregion

#pragma region GunCamera
	{
		shared_ptr<GunCamera> gunCamera = make_shared<GunCamera>();
		gunCamera->SetName(L"Gun_Camera");
		gunCamera->SetTransform(make_shared<Transform>());
		gunCamera->SetCamera(make_shared<Camera>());
		gunCamera->GetCamera()->SetFOV(60.f);
		gunCamera->GetCamera()->SetFar(1000.f);
		gunCamera->GetCamera()->SetCullingMaskAll(); // 다 끄고
		gunCamera->GetCamera()->SetCullingMaskLayerOnOff(GET_SINGLE(SceneMgr)->LayerNameToIndex(L"Gun"), false); // Gun만 찍음

		// Main_Camera의 Transform을 따라가도록 설정
		shared_ptr<GameObject> mainCamera = scene->GetPlayerCamera()->GetGameObject();
		gunCamera->GetTransform()->SetParent(mainCamera->GetTransform());

		scene->AddGameObject(gunCamera);
	}
#pragma endregion

#pragma region UI_Camera
	{
		shared_ptr<UICamera> camera = make_shared<UICamera>();
		camera->SetName(L"UI_Camera");
		camera->SetTransform(make_shared<Transform>());
		camera->SetCamera(make_shared<Camera>());
		camera->GetTransform()->SetLocalPosition(Vec3(0.f, 0.f, 0.f));
		camera->GetCamera()->SetProjectionType(PROJECTION_TYPE::ORTHOGRAPHIC);
		uint8 layerIndex = GET_SINGLE(SceneMgr)->LayerNameToIndex(L"UI");
		camera->GetCamera()->SetCullingMaskAll(); // 다 끄고
		camera->GetCamera()->SetCullingMaskLayerOnOff(layerIndex, false); // UI만 찍음
		scene->AddGameObject(camera);
	}
#pragma endregion


#pragma region SkyBox
	{
		shared_ptr<GameObject> skybox = make_shared<GameObject>();
		skybox->SetTransform(make_shared<Transform>());
		skybox->SetCheckFrustum(false);
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		{
			shared_ptr<Mesh> sphereMesh = GET_SINGLE(Resources)->LoadSphereMesh();
			meshRenderer->SetMesh(sphereMesh);
		}
		{
			shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"Skybox");
			shared_ptr<Texture> texture = GET_SINGLE(Resources)->Load<Texture>(L"Sky02", L"..\\Resources\\Texture\\Sky02.png");
			shared_ptr<Material> material = make_shared<Material>();
			material->SetShader(shader);
			material->SetTexture(0, texture);
			meshRenderer->SetMaterial(material);
		}
		skybox->SetMeshRenderer(meshRenderer);
		scene->AddGameObject(skybox);
	}
#pragma endregion

#pragma region UI_Test
	for (int32 i = 0; i < 6; i++)
	{
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		obj->SetLayerIndex(GET_SINGLE(SceneMgr)->LayerNameToIndex(L"UI")); // UI
		obj->SetTransform(make_shared<Transform>());
		obj->GetTransform()->SetLocalScale(Vec3(100.f, 100.f, 100.f));
		obj->GetTransform()->SetLocalPosition(Vec3(-350.f + (i * 120), 250.f, 500.f));
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		{
			shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
			meshRenderer->SetMesh(mesh);
		}
		{
			shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"Texture");

			shared_ptr<Texture> texture;
			if (i < 3)
				texture = gameFramework->GetRTGroup(RENDER_TARGET_GROUP_TYPE::G_BUFFER)->GetRTTexture(i);
			else if (i < 5)
				texture = gameFramework->GetRTGroup(RENDER_TARGET_GROUP_TYPE::LIGHTING)->GetRTTexture(i - 3);
			else
				texture = gameFramework->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SHADOW)->GetRTTexture(0);

			shared_ptr<Material> material = make_shared<Material>();
			material->SetShader(shader);
			material->SetTexture(0, texture);
			meshRenderer->SetMaterial(material);
		}
		obj->SetMeshRenderer(meshRenderer);
		scene->AddGameObject(obj);
	}
#pragma endregion

#pragma region Load_UI_Image
	{
		LoadUIImage(scene);
	}
#pragma endregion

#pragma region Local Player
	{
		vector<shared_ptr<LocalPlayer>> localPlayers;
		shared_ptr<LocalPlayer> localPlayer = make_shared<LocalPlayer>();
		localPlayer->SetName(L"LocalPlayer");
		localPlayer->SetTransform(make_shared<Transform>());
		localPlayer->GetTransform()->SetLocalPosition(Vec3(1185.f, 140.f, 473.f));
		localPlayer->SetCheckFrustum(false);

		localPlayers.push_back(localPlayer);

		scene->GetPlayerCamera()->GetTransform()->SetParent(localPlayer->GetTransform());
		scene->AddGameObject(localPlayer);

		// 로컬 플레이어 설정
		vector<shared_ptr<Player>> players;
		players.reserve(localPlayers.size());

		std::transform(localPlayers.begin(), localPlayers.end(), std::back_inserter(players),
			[](const shared_ptr<LocalPlayer>& mp) {
				return static_pointer_cast<Player>(mp); // 업캐스팅
			});

		scene->SetLocalPlayer(players);
	}
#pragma endregion

#pragma region M4A1
	{
		shared_ptr<MeshData> meshData = GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\M4A1.fbx");

		vector<shared_ptr<M4A1>> gameObjects = meshData->InstantiateAs<M4A1>();
		uint8 gunLayer = GET_SINGLE(SceneMgr)->LayerNameToIndex(L"Gun");

		for (auto& gameObject : gameObjects)
		{
			if (gameObject->GetMeshRenderer())
			{
				auto mat = gameObject->GetMeshRenderer()->GetMaterial()->Clone(); // 기존 머티리얼 복사
				auto shader = GET_SINGLE(Resources)->Get<Shader>(L"Forward"); // FORWARD 타입 셰이더

				mat->SetShader(shader);
				gameObject->GetMeshRenderer()->SetMaterial(mat);
			}
			gameObject->SetLayerIndex(gunLayer);
			gameObject->SetActive(false);

			scene->AddGameObject(gameObject);
		}

		gameObjects[0]->SetName(L"M4A1");
	}
#pragma endregion

#pragma region AK47
	{
		shared_ptr<MeshData> meshData = GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\AK74.fbx");

		vector<shared_ptr<AK47>> gameObjects = meshData->InstantiateAs<AK47>();
		uint8 gunLayer = GET_SINGLE(SceneMgr)->LayerNameToIndex(L"Gun");

		for (auto& gameObject : gameObjects)
		{
			if (gameObject->GetMeshRenderer())
			{
				auto mat = gameObject->GetMeshRenderer()->GetMaterial()->Clone(); // 기존 머티리얼 복사
				auto shader = GET_SINGLE(Resources)->Get<Shader>(L"Forward"); // FORWARD 타입 셰이더

				mat->SetShader(shader);
				gameObject->GetMeshRenderer()->SetMaterial(mat);
			}
			gameObject->SetLayerIndex(gunLayer);
			gameObject->SetActive(true);

			scene->AddGameObject(gameObject);
		}

		gameObjects[0]->SetName(L"AK47");
	}
#pragma endregion

#pragma region Directional Light
	{
		shared_ptr<LightObject> light = make_shared<LightObject>();
		light->SetTransform(make_shared<Transform>());
		light->GetTransform()->SetLocalPosition(Vec3(1185.f, 4000.f, 473.f));
		light->SetLight(make_shared<Light>());
		light->GetLight()->SetLightDirection(Vec3(0.f, -1.f, 0.f));
		light->GetLight()->SetLightType(LIGHT_TYPE::DIRECTIONAL_LIGHT);
		light->GetLight()->SetDiffuse(Vec3(1.f, 1.f, 1.f));
		light->GetLight()->SetAmbient(Vec3(0.1f, 0.1f, 0.1f));
		light->GetLight()->SetSpecular(Vec3(0.1f, 0.1f, 0.1f));

		scene->AddGameObject(light);
	}
#pragma endregion

//#pragma region Map
//	{
//		shared_ptr<MeshData> meshData = GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\Factory1Items.fbx");
//		vector<shared_ptr<GameObject>> gameObjects = meshData->Instantiate(ColliderType::OBB);
//
//		for (auto& gameObject : gameObjects)
//		{
//			gameObject->SetName(L"Map");
//			//gameObject->SetStatic(true);
//			gameObject->GetTransform()->SetLocalScale(Vec3(100.f, 100.f, 100.f));
//			gameObject->GetTransform()->SetLocalRotation(Vec3(0.f, 0.f, 0.f));
//			//gameObject->GetTransform()->SetParent(t->GetTransform());
//			scene->AddGameObject(gameObject);
//		}
//
//		//GET_SINGLE(JsonMgr)->SaveMapCollider(L"..\\Resources\\Json\\MapCollider.json", gameObjects);
//	}
//#pragma endregion

	GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\Soldado.fbx");
	//GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\NormalZombie.fbx");
	//GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\PoliceZombie.fbx");
	GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\EliteZombie.fbx");

	return scene;
}

void SceneMgr::LoadUIImage(shared_ptr<Scene> scene)
{
	// 조준선 UI 생성
	GET_SINGLE(UIMgr)->CreateImageUI(
		L"Crosshair",
		L"..\\Resources\\Texture\\Crosshair\\crosshair01.png",
		Vec2(0.f, 0.f), // 화면 중앙
		Vec2(50.f, 50.f), // 크기
		scene
	);


	// 총 패널 UI 생성
	GET_SINGLE(UIMgr)->CreateRectangleUI(
		L"GunPanel_1",
		Vec2(490.f, -360.f),
		Vec2(300.f, 50.f),
		Vec4(0.5f, 0.5f, 0.5f, 0.5f), // 반투명 검정색
		scene
	);

	GET_SINGLE(UIMgr)->CreateImageUI(
		L"AK47",
		L"..\\Resources\\Texture\\Icon\\Gun\\AK47 실루엣(흰색).png",
		Vec2(520.f, -360.f), 
		Vec2(165.f, 50.f),
		scene
	);
	GET_SINGLE(UIMgr)->CreateImageUI(
		L"소총탄",
		L"..\\Resources\\Texture\\Icon\\Bullet\\소총탄.png",
		Vec2(410.f, -360.f), 
		Vec2(40.f, 40.f),
		scene
	);

	// 플레이어1 정보 출력
	GET_SINGLE(UIMgr)->CreateRectangleUI(
		L"PlayerPanel_1",					// 이름
		Vec2(-500.f, -360.f),				// 위치
		Vec2(300.f, 50.f),					// 크기
		Vec4(0.5f, 0.5f, 0.5f, 0.5f),		// 색상
		scene
	);

	GET_SINGLE(UIMgr)->CreateRectangleUI(
		L"PlayerPanel_1_Max_Hp",			// 이름
		Vec2(-495.f, -370.f),				// 위치
		Vec2(270.f, 20.f),					// 크기
		Vec4(0.0f, 0.0f, 0.0f, 1.f),		// 색상
		scene
	);
	GET_SINGLE(UIMgr)->CreateRectangleUI(
		L"PlayerPanel_1_HP",				// 이름
		Vec2(-495.f, -370.f),				// 위치
		Vec2(270.f, 20.f),					// 크기
		Vec4(1.f, 0.0f, 0.0f, 1.f),			// 색상
		scene
	);


	// 플레이어2 정보 출력
	GET_SINGLE(UIMgr)->CreateRectangleUI(
		L"PlayerPanel_2",					// 이름
		Vec2(-500.f, -300.f),				// 위치
		Vec2(300.f, 50.f),					// 크기
		Vec4(0.5f, 0.5f, 0.5f, 0.5f),		// 색상
		scene
	);

	GET_SINGLE(UIMgr)->CreateRectangleUI(
		L"PlayerPanel_2_Max_Hp",			// 이름
		Vec2(-495.f, -310.f),				// 위치
		Vec2(270.f, 20.f),					// 크기
		Vec4(0.0f, 0.0f, 0.0f, 1.f),		// 색상
		scene
	);
	GET_SINGLE(UIMgr)->CreateRectangleUI(
		L"PlayerPanel_2_HP",				// 이름
		Vec2(-495.f, -310.f),				// 위치
		Vec2(270.f, 20.f),					// 크기
		Vec4(1.f, 0.0f, 0.0f, 1.f),			// 색상
		scene
	);


	// 플레이어3 정보 출력
	GET_SINGLE(UIMgr)->CreateRectangleUI(
		L"PlayerPanel_3",					// 이름
		Vec2(-500.f, -240.f),				// 위치
		Vec2(300.f, 50.f),					// 크기
		Vec4(0.5f, 0.5f, 0.5f, 0.5f),		// 색상
		scene
	);

	GET_SINGLE(UIMgr)->CreateRectangleUI(
		L"PlayerPanel_3_Max_Hp",			// 이름
		Vec2(-495.f, -250.f),				// 위치
		Vec2(270.f, 20.f),					// 크기
		Vec4(0.0f, 0.0f, 0.0f, 1.f),		// 색상
		scene
	);
	GET_SINGLE(UIMgr)->CreateRectangleUI(
		L"PlayerPanel_3_HP",				// 이름
		Vec2(-495.f, -250.f),				// 위치
		Vec2(270.f, 20.f),					// 크기
		Vec4(1.f, 0.0f, 0.0f, 1.f),			// 색상
		scene
	);

	


	//for (size_t i = 0; i < 20; i++)
	//{
	//	// 플레이어 정보 출력
	//	GET_SINGLE(UIMgr)->CreateImageUI(
	//		L"소총탄",
	//		L"..\\Resources\\Texture\\Icon\\Bullet\\소총탄.png",
	//		Vec2(410.f, -360.f),
	//		Vec2(40.f, 40.f),
	//		scene
	//	);
	//}

	
}
