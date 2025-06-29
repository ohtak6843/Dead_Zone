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

#include "UIMgr.h"

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
	uint8 backbufferindex = gameFramework->GetCurrBackBufferIndex();
	shared_ptr<D3D11On12Device> device = gameFramework->GetD3D11on12Device();
	D2D1_SIZE_F rtSize = device->GetD3D11On12RT(backbufferindex)->GetSize();
	//D2D1_RECT_F textRect = D2D1::RectF(0, 0, rtSize.width, rtSize.height);

	// Acquire our wrapped render target resource for the current back buffer.
	device->GetD3D11on12Device()->AcquireWrappedResources(device->GetWrappedBackBuffer(backbufferindex).GetAddressOf(), 1);
	// Render text directly to the back buffer.
	device->GetD2DDeviceContext()->SetTarget(device->GetD3D11On12RT(backbufferindex).Get());
	device->GetSolidColorBrush()->SetColor(D2D1::ColorF(D2D1::ColorF::White));
	device->GetD2DDeviceContext()->BeginDraw();

	// TODO: UI 렌더링
	//if (_activeScene)
	//	_activeScene->RenderUI();

	static float elapsedTime = 0.f;
	if (DELTA_TIME < 1.f)
		elapsedTime += DELTA_TIME;
	if (_activeScene && GET_SINGLE(SceneMgr)->GetSceneType() != SCENE_TYPE::LOADING)
	{
		// 총알 UI
		Vec2 pivot = {
			static_cast<float>(gameFramework->GetWindow().width - 100),
			static_cast<float>(gameFramework->GetWindow().height - 50) };
		D2D1_RECT_F textRect = D2D1::RectF(pivot.x - 100, pivot.y - 100, pivot.x + 100, pivot.y + 100);
		int32 currentAmmo = 0;
		
		int32 gunType = static_pointer_cast<LocalPlayer>(_activeScene->FindGameObject(L"LocalPlayer"))->getGunType();
		if (gunType == 0)
			currentAmmo = static_pointer_cast<M4A1>(_activeScene->FindGameObject(L"M4A1"))->GetCurrentAmmo();
		else if (gunType == 1)
			currentAmmo = static_pointer_cast<AK47>(_activeScene->FindGameObject(L"AK47"))->GetCurrentAmmo();
		
		std::wstringstream wss1;
		wss1 << std::fixed << std::setprecision(2) << currentAmmo;
		wstring text = L"탄창: ";
		text += wss1.str();
		device->GetD2DDeviceContext()->DrawTextW(
			text.c_str(),
			static_cast<uint32>(text.size()),
			device->GetTextFormat().Get(),
			&textRect,
			device->GetSolidColorBrush().Get());

		// 타이머 UI
		//pivot = { static_cast<float>(gameFramework->GetWindow().width / 2), 50.f };
		//D2D1_RECT_F textRect2 = D2D1::RectF(pivot.x - 100, pivot.y - 100, pivot.x + 100, pivot.y + 100);

		//wstring text2 = L"시간 : ";
		//std::wstringstream wss;
		//wss << std::fixed << std::setprecision(2) << elapsedTime;
		//text2 += wss.str();
		//device->GetD2DDeviceContext()->DrawTextW(
		//	text2.c_str(),
		//	static_cast<uint32>(text2.size()),
		//	device->GetTextFormat().Get(),
		//	&textRect2,
		//	device->GetSolidColorBrush().Get());

		// 체력 UI
		pivot = { 100.f, static_cast<float>(gameFramework->GetWindow().height - 50) };
		D2D1_RECT_F textRect3 = D2D1::RectF(pivot.x - 100, pivot.y - 100, pivot.x + 100, pivot.y + 100);

		wstring text3 = L"HP : 100";
		device->GetD2DDeviceContext()->DrawTextW(
			text3.c_str(),
			static_cast<uint32>(text3.size()),
			device->GetTextFormat().Get(),
			&textRect3,
			device->GetSolidColorBrush().Get());

		pivot = { 100.f, static_cast<float>(gameFramework->GetWindow().height / 2) };
		D2D1_RECT_F textRect4 = D2D1::RectF(pivot.x - 100, pivot.y - 200, pivot.x + 100, pivot.y + 200);

		//std::wstringstream wss;
		//wstring text4 = L"X : ";
		//Vec3 mainCameraPos = _activeScene->GetPlayerCamera()->GetTransform()->GetWorldPosition();
		//wss.str(L"");
		//wss.clear();
		//wss << std::fixed << std::setprecision(2) << mainCameraPos.x;
		//text4 += wss.str();
		//text4 += L"\nY : ";
		//wss.str(L"");
		//wss.clear();
		//wss << std::fixed << std::setprecision(2) << mainCameraPos.y;
		//text4 += wss.str();
		//text4 += L"\nZ : ";
		//wss.str(L"");
		//wss.clear();
		//wss << std::fixed << std::setprecision(2) << mainCameraPos.z;
		//text4 += wss.str();
		//device->GetD2DDeviceContext()->DrawTextW(
		//	text4.c_str(),
		//	static_cast<uint32>(text4.size()),
		//	device->GetTextFormat().Get(),
		//	&textRect4,
		//	device->GetSolidColorBrush().Get());
	}

	device->GetD2DDeviceContext()->EndDraw();
	// Release our wrapped render target resource. Releasing 
	// transitions the back buffer resource to the state specified
	// as the OutState when the wrapped resource was created.
	device->GetD3D11on12Device()->ReleaseWrappedResources(device->GetWrappedBackBuffer(backbufferindex).GetAddressOf(), 1);

	// Flush to submit the 11 command list to the shared command queue.
	device->GetD3D11DeviceContext()->Flush();
}

void SceneMgr::LoadScene(wstring sceneName)
{
	GET_SINGLE(SceneMgr)->SetSceneType(SCENE_TYPE::LOADING);
	_activeScene = LoadLoadingScene();
	_activeScene->Awake();
	_activeScene->Start();
	gameFramework->Update();

	GET_SINGLE(SceneMgr)->SetSceneType(SCENE_TYPE::STAGE01);
	_activeScene = LoadStage01();

	_activeScene->Awake();
	_activeScene->Start();
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
	SetLayerName(1, L"Gun"); // 총 UI 별도 처리
	SetLayerName(2, L"UI");
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

#pragma region Crosshair
	{
		GET_SINGLE(UIMgr)->CreateImageUI(
			L"Crosshair",
			L"..\\Resources\\Texture\\Crosshair\\crosshair01.png",
			Vec2(0.f, 0.f), // 화면 중앙
			Vec2(50.f, 50.f), // 크기
			scene
		);
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

#pragma region Map
	{
		shared_ptr<MeshData> meshData = GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\Factory1Items.fbx");
		vector<shared_ptr<GameObject>> gameObjects = meshData->Instantiate(ColliderType::OBB);

		for (auto& gameObject : gameObjects)
		{
			gameObject->SetName(L"Map");
			//gameObject->SetStatic(true);
			gameObject->GetTransform()->SetLocalScale(Vec3(100.f, 100.f, 100.f));
			gameObject->GetTransform()->SetLocalRotation(Vec3(0.f, 0.f, 0.f));
			//gameObject->GetTransform()->SetParent(t->GetTransform());
			scene->AddGameObject(gameObject);
		}

		//GET_SINGLE(JsonMgr)->SaveMapCollider(L"..\\Resources\\Json\\MapCollider.json", gameObjects);
	}
#pragma endregion

	GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\Soldado.fbx");
	GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\NormalZombie.fbx");

	return scene;
}