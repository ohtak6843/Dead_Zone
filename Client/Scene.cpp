#include "pch.h"
#include "Scene.h"
#include "GameObject.h"
#include "Camera.h"
#include "Framework.h"
#include "ConstantBuffer.h"
#include "Light.h"
#include "Resources.h"
#include "Timer.h"

#include "Transform.h"
#include "MeshRenderer.h"
#include "MeshData.h"
#include "SphereCollider.h"
#include "Animator.h"
#include "RenderPass.h"

#include "CameraObject.h"
#include "LightObject.h"

#include "Player.h"
#include "MultiPlayer.h"

#include "Zombie.h"
#include "PoliceZombie.h"
#include "EliteZombie.h"

#include "TP_AK47.h"

#include "SceneMgr.h"
#include "FmodMgr.h"

#include "..//echoserver//protocol.h"

extern WindowInfo GWindowInfo;

Scene::Scene()
{
	_renderPass = make_shared<RenderPass>();
}

Scene::~Scene()
{
}

void Scene::Release()
{
	_gameObjects.clear();
	_players.clear();
	_zombies.clear();
	_jumpStates.clear();
	
	_renderPass.reset();
}

void Scene::Awake()
{
	for (const shared_ptr<GameObject>& gameObject : _gameObjects)
	{
		gameObject->Awake();
	}
}

void Scene::Start()
{
	for (const shared_ptr<GameObject>& gameObject : _gameObjects)
	{
		gameObject->Start();
	}
}

void Scene::Update()
{
	for (const shared_ptr<GameObject>& gameObject : _gameObjects)
	{
		if (!gameObject->IsActive()) continue;
		gameObject->Update();
	}
}

void Scene::LateUpdate()
{
	const float gravity = 9.8f;

	for (auto& [id, group] : _players) {
		auto & root = group[0];
		auto itJS = _jumpStates.find(id);
		if (itJS == _jumpStates.end() || !itJS->second.isJumping)
			 continue;
		auto & js = itJS->second;
		Vec3 pos = root->GetTransform()->GetLocalPosition();
		pos.y += js.verticalVel * DELTA_TIME;
		js.verticalVel -= gravity * DELTA_TIME;
		if (pos.y <= 0.0f) {
			pos.y = 0.0f;
			js.isJumping = false;
			js.verticalVel = 0.0f;
		}
		root->GetTransform()->SetLocalPosition(pos);
	}

	for (const shared_ptr<GameObject>& gameObject : _gameObjects)
	{
		if (!gameObject->IsActive()) continue;
		gameObject->LateUpdate();
	}
}

void Scene::FinalUpdate()
{
	for (const shared_ptr<GameObject>& gameObject : _gameObjects)
	{
		if (!gameObject) continue; // 방어 코드, 크러시 원인 찾기 전까지 임시
		if (!gameObject->IsActive()) continue;
		gameObject->FinalUpdate();
	}
}

shared_ptr<class Camera> Scene::GetMainCamera()
{
	return _renderPass->GetMainCamera();
}

shared_ptr<Camera> Scene::GetPlayerCamera()
{
	return _renderPass->GetPlayerCamera();
}

shared_ptr<class Camera> Scene::GetGunCamera()
{
	return _renderPass->GetGunCamera();
}

void Scene::Render()
{
	_renderPass->Render();
}

void Scene::RenderUI()
{

}

void Scene::AddGameObject(shared_ptr<GameObject> gameObject)
{
	switch (gameObject->GetGameObjectType())
	{
	case GAMEOBJECT_TYPE::CAMERA:
	{
		auto cameraObject = static_pointer_cast<CameraObject>(gameObject);
		if (cameraObject->GetCamera() != nullptr)
		{
			_renderPass->AddCamera(cameraObject->GetCamera());
		}
		break;
	}
	case GAMEOBJECT_TYPE::LIGHT:
	{
		auto lightObject = static_pointer_cast<LightObject>(gameObject);
		if (lightObject->GetLight() != nullptr)
		{
			_renderPass->AddLight(lightObject->GetLight());
		}
		break;
	}
	default:
		break;
	}

	_gameObjects.push_back(gameObject);
}

void Scene::RemoveGameObject(shared_ptr<GameObject> gameObject)
{
	switch (gameObject->GetGameObjectType())
	{
	case GAMEOBJECT_TYPE::CAMERA:
	{
		auto cameraObject = static_pointer_cast<CameraObject>(gameObject);
		if (cameraObject->GetCamera() != nullptr)
		{
			_renderPass->RemoveCamera(cameraObject->GetCamera());
		}
		break;
	}
	case GAMEOBJECT_TYPE::LIGHT:
	{
		auto lightObject = static_pointer_cast<LightObject>(gameObject);
		if (lightObject->GetLight() != nullptr)
		{
			_renderPass->RemoveLight(lightObject->GetLight());
		}
		break;
	}
	default:
		break;
	}

	auto findIt = std::find(_gameObjects.begin(), _gameObjects.end(), gameObject);
	if (findIt != _gameObjects.end())
		_gameObjects.erase(findIt);
}

shared_ptr<GameObject> Scene::FindGameObject(const wstring& name)
{
	for (const shared_ptr<GameObject>& gameObject : _gameObjects)
	{
		if (gameObject->GetName() == name)
			return gameObject;
	}

	return nullptr;
}

void Scene::ActiveGameObject(const wstring& name, bool flag)
{
	FindGameObject(name)->SetActive(flag);
}

void Scene::SetLocalPlayer(vector<shared_ptr<Player>>& player)
{
	_localPlayer = std::move(player);
}

void Scene::AddPlayer(sc_packet_player_info* packet)
{
	//if (_players.size() >= 2)
		//return;

	Vec3 position = Vec3(packet->position.x, packet->position.y, packet->position.z);

	shared_ptr<MeshData> meshData = GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\Soldado.fbx");

	vector<shared_ptr<MultiPlayer>> gameObjects = meshData->InstantiateAs<MultiPlayer>(ColliderType::OBB);

	// 플레이어 모델
	for (auto& gameObject : gameObjects)
	{
		gameObject->SetName(L"Player");
		gameObject->SetState(PLAYER_STATE::IDLE);
		AddGameObject(gameObject);
	}

	gameObjects[0]->SetID(static_cast<uint32_t>(packet->playerId));
	gameObjects[0]->GetTransform()->SetLocalPosition(position);
	gameObjects[0]->GetTransform()->SetLocalRotation(Vec3(-90.0f, 180.f, 0.0f));

	for (int i = 1; i < gameObjects.size(); i++)
	{
		gameObjects[i]->GetTransform()->SetParent(gameObjects[0]->GetTransform());
	}

	// 플레이어 씬에 추가
	vector<shared_ptr<Player>> players;
	players.reserve(gameObjects.size());

	std::transform(gameObjects.begin(), gameObjects.end(), std::back_inserter(players),
		[](const shared_ptr<MultiPlayer>& mp) {
			return static_pointer_cast<Player>(mp); // 업캐스팅
		});
	_players[packet->playerId] = std::move(players);

	// 총 모델
	shared_ptr<MeshData> guns = GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\AK47.fbx");
	vector<shared_ptr<TP_AK47>> gunObjects = guns->InstantiateAs<TP_AK47>();

	for (auto& gunObject : gunObjects)
	{
		gunObject->SetName(L"TP_AK47");
		gunObject->GetTransform()->SetParent(gameObjects[0]->GetTransform());
		gunObject->SetParentObject(gameObjects[0]);
		AddGameObject(gunObject);
	}

	// 플레이어에 총 추가
	vector<shared_ptr<Gun>> tempGuns;
	tempGuns.reserve(gunObjects.size());

	std::transform(gunObjects.begin(), gunObjects.end(), std::back_inserter(tempGuns),
		[](const shared_ptr<TP_AK47>& mp) {
			return static_pointer_cast<Gun>(mp); // 업캐스팅
		});
	gameObjects[0]->AddGun(tempGuns);
}

void Scene::MovePlayer(sc_packet_move* packet)
{
	//// 사운드 재생
	//bool flag = GET_SINGLE(FmodMgr)->CheckPlaying(SOUND_TYPE::PLAYER_RUN);
	//if (flag == false)
	//	GET_SINGLE(FmodMgr)->PlaySound(SOUND_TYPE::PLAYER_RUN);

	Vec3 position = Vec3(packet->position.x, packet->position.y, packet->position.z);
	Vec3 look = Vec3(packet->look.x, packet->look.y, packet->look.z);

	if (packet->playerId == GWindowInfo.local) {
		auto& root = _localPlayer[0];  
		if (root) {
			shared_ptr<Transform> rootTransform = root->GetTransform();
			rootTransform->SetLocalPosition(position);
			rootTransform->LookAt(look);

			//Vec3 rotation = rootTransform->GetLocalRotation();
			//rotation.x = -90.f;
			//rotation.y += 180.f;
			//rootTransform->SetLocalRotation(rotation);
		}
		return;
	}

	for (auto& [id, group] : _players) {
		auto& root = group[0];
		if (id == packet->playerId) {
			shared_ptr<Transform> rootTransform = root->GetTransform();
			rootTransform->SetLocalPosition(position);
			rootTransform->LookAt(look);

			Vec3 rotation = rootTransform->GetLocalRotation();
			rotation.x = -90.f;
			rotation.y += 180.f;
			rootTransform->SetLocalRotation(rotation);
			return;
		}
	}
}

void Scene::JumpPlayer(sc_packet_jump* packet)
{
	uint32_t id = static_cast<uint32_t>(packet->playerId);
	auto& js = _jumpStates[id];
	js.isJumping = true;
	js.verticalVel = packet->initVelocity;
}

void Scene::LandPlayer(sc_packet_land* packet)
{
	uint32_t id = static_cast<uint32_t>(packet->playerId);
	auto& js = _jumpStates[id];
	js.isJumping = false;
	js.verticalVel = 0.0f;
}

void Scene::AnimatePlayer(sc_packet_state* packet)
{
	PLAYER_STATE state = static_cast<PLAYER_STATE>(packet->state);
	for (auto& [id, group] : _players) {
		auto& root = group[0];
		if (id == packet->playerId) {
			for (auto& part : group) {
				part->SetState(state);
			}
			return;
		}
	}
}

void Scene::RemovePlayer(sc_packet_player_leave* packet)
{
	uint32_t leftId = static_cast<uint32_t>(packet->playerId);

	for (auto& [id, group] : _players)
	{
		if (id == packet->playerId)
		{
			auto& guns = group[0]->GetGuns();
			for (auto& gunGroup : guns) {
				for (auto& gun : gunGroup) {
					RemoveGameObject(gun);
				}
			}

			guns.clear();

			for (auto& part : group)
				RemoveGameObject(part);

			_players.erase(id);
		}
	}
}

void Scene::ClearPlayers()
{
	for (auto& [id, group] : _players) {
		for (auto& obj : group) {
			RemoveGameObject(obj);
		}
	}
	_players.clear();
}

void Scene::ClearZombies()
{
	// _zombies는 vector< vector< shared_ptr<GameObject> > >
	for (auto& group : _zombies) {
		for (auto& part : group) {
			RemoveGameObject(part);
		}
	}
	_zombies.clear();
}

void Scene::AddZombie(sc_packet_spawn_zombie* packet)
{
	Vec3 position = Vec3(packet->position.x, packet->position.y, packet->position.z);

	shared_ptr<MeshData> meshData = GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\NormalZombie.fbx");
	vector<shared_ptr<Zombie>> zombieObjects = meshData->InstantiateAs<Zombie>(ColliderType::OBB);

	//shared_ptr<MeshData> meshData = GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\PoliceZombie.fbx");
	//vector<shared_ptr<PoliceZombie>> zombieObjects = meshData->InstantiateAs<PoliceZombie>(ColliderType::OBB);

	//shared_ptr<MeshData> meshData = GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\EliteZombie.fbx");
	//vector<shared_ptr<EliteZombie>> zombieObjects = meshData->InstantiateAs<EliteZombie>(ColliderType::OBB);

	for (const auto& zombieObject : zombieObjects)
	{
		zombieObject->SetName(L"Zombie");
		zombieObject->SetState(ZOMBIE_STATE::IDLE);
	}

	zombieObjects[0]->SetID(static_cast<uint32_t>(packet->zombieId));
	zombieObjects[0]->GetTransform()->SetLocalPosition(position);
	zombieObjects[0]->GetTransform()->SetLocalRotation(Vec3(-90.0f, 180.f, 0.0f));

	for (int i = 1; i < zombieObjects.size(); i++)
	{
		zombieObjects[i]->GetTransform()->SetParent(zombieObjects[0]->GetTransform());
	}

	// 씬에 추가
	for (auto& zombie : zombieObjects) {
		AddGameObject(zombie);
	}
	// 업 캐스팅
	vector<shared_ptr<Zombie>> castedZombies;
	for (const auto& pz : zombieObjects) {
		castedZombies.push_back(static_pointer_cast<Zombie>(pz));
	}

	_zombies.push_back(castedZombies);
}

void Scene::MoveZombie(sc_packet_zombie_move* packet)
{
	Vec3 position = Vec3(packet->position.x, packet->position.y, packet->position.z);
	Vec3 look = Vec3(packet->dx, 0.f, packet->dz);
	uint32_t zid = static_cast<uint32_t>(packet->zombieId);

	for (auto& group : _zombies) {
		auto& root = group[0];
		if (root->GetID() == zid) {
			shared_ptr<Transform> rootTransform = root->GetTransform();
			rootTransform->SetLocalPosition(position);
			rootTransform->LookAt(look);

			Vec3 rotation = rootTransform->GetLocalRotation();
			rotation.x = -90.f;
			rotation.y += 180.f;
			rootTransform->SetLocalRotation(rotation);
			return;
		}
	}
}

void Scene::AnimateZombie(sc_packet_zombie_state* packet)
{
	ZOMBIE_STATE state = static_cast<ZOMBIE_STATE>(packet->state);
	for (auto& group : _zombies) {
		auto& root = group[0];
		if (root->GetID() == packet->zombieId) {
			for (auto& part : group) {
				part->SetState(state);
			}
			return;
		}
	}
}

void Scene::DieZombie(sc_packet_zombie_die* pkt)
{
	uint32_t id = static_cast<uint32_t>(pkt->zombieId);

	// _zombies: vector< vector< shared_ptr<GameObject> > >
	for (auto& group : _zombies) {
		if (group.empty() || group[0]->GetID() != id)
			continue;

		// 1) 각 파트에 대해 DIE 상태로 전환 → 애니메이션 재생
		for (auto& part : group) {
			part->SetState(ZOMBIE_STATE::DIE);
		}

		auto anim = group[0]->GetAnimator();
		int32 idx = anim->GetCurrentClipIndex();
		float deathAnimDuration = anim->GetAnimDuration(idx);
		GET_SINGLE(Timer)->SetTimeout([this, id]() {
			RemoveZombieById(id);
		}, deathAnimDuration);
		// 한 번 처리했으면 루프 탈출
		break;
	}
}

// 기존 RemoveZombieById는 그대로 사용
void Scene::RemoveZombieById(uint32_t zombieId)
{
	for (auto it = _zombies.begin(); it != _zombies.end(); ++it) {
		auto& group = *it;
		if (group.empty() || group[0]->GetID() != zombieId)
			continue;

		// 씬 오브젝트에서 제거
		for (auto& part : group) {
			RemoveGameObject(part);
		}
		// 벡터에서 그룹 삭제
		_zombies.erase(it);
		return;
	}
}

void Scene::ApplySnapshot(sc_packet_snapshot* packet)
{
	for (int i = 0; i < packet->count; ++i) {
		auto& e = packet->entries[i];
		uint32_t playerId = static_cast<uint32_t>(e.playerId);

		if (playerId == gameFramework->GetWindow().local) {
			auto cam = GetMainCamera();
			auto camGO = cam->GetGameObject();
			auto camTrans = camGO->GetTransform();

			Vec3 predicted = camTrans->GetLocalPosition();
			Vec3 serverPos{ e.position.x, e.position.y+140.f, e.position.z };

			Vec3 delta = serverPos - predicted;
			const float snapThreshold = 1.0f;   
			const float lerpRatio = 0.1f;   

			if (delta.Length() > snapThreshold) {
				camTrans->SetLocalPosition(serverPos);
			}
			else {
				camTrans->SetLocalPosition(predicted + delta * lerpRatio);
			}
			continue;  
		}

		shared_ptr<GameObject> rootObj;
		for (auto& [id, group] : _players) {
			if (id == playerId) {
				rootObj = group[0];
				break;
			}
		}
		if (!rootObj) continue;

		auto itJS = _jumpStates.find(playerId);
		bool remoteJumping = (itJS != _jumpStates.end() && itJS->second.isJumping);

		Vec3 pos = rootObj->GetTransform()->GetLocalPosition();

		pos.x = e.position.x;
		pos.z = e.position.z;

		if (!remoteJumping) {
			pos.y = e.position.y;
		}
		rootObj->GetTransform()->SetLocalPosition(pos);

		// 3) 회전도 보정
		/*Vec3 rot = obj->GetTransform()->GetLocalRotation();
		rot.y = e.yaw;
		obj->GetTransform()->SetLocalRotation(rot);*/
	}
}