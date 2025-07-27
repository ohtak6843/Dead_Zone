#pragma once

class GameObject;
class Camera;
class Light;

class CameraObject;
class LightObject;

class Player;
class Zombie;

struct JumpState {
	bool  isJumping = false;
	float verticalVel = 0.0f;
};

class Scene
{
public:
	Scene();
	virtual ~Scene();

	virtual void LoadResources() {}

	virtual void Init() {}
	virtual void Release();

	virtual void Awake();
	virtual void Start();
	virtual void Update();
	virtual void LateUpdate();
	virtual void FinalUpdate();

public:
	shared_ptr<Camera> GetMainCamera();
	shared_ptr<Camera> GetPlayerCamera();
	shared_ptr<Camera> GetGunCamera();

	void Render();
	void RenderUI();
	
private:

public:
	void AddGameObject(shared_ptr<GameObject> gameObject);
	void RemoveGameObject(shared_ptr<GameObject> gameObject);

	const vector<shared_ptr<GameObject>>& GetGameObjects() { return _gameObjects; }

	const unordered_map<long long, vector<shared_ptr<Player>>>& GetPlayers() { return _players; }
	const vector<vector<shared_ptr<Zombie>>>& GetZombies() { return _zombies; }

public:
	shared_ptr<GameObject> FindGameObject(const wstring& name);
	void ActiveGameObject(const wstring& name, bool flag);

public:
	void SetLocalPlayer(vector<shared_ptr<Player>>& player);

	void AddPlayer(struct sc_packet_player_info* packet);
	void MovePlayer(struct sc_packet_move* packet);
	void JumpPlayer(struct sc_packet_jump* packet);
	void LandPlayer(struct sc_packet_land* packet);
	void AnimatePlayer(struct sc_packet_state* packet);
	void RemovePlayer(struct sc_packet_player_leave* packet);
	void UpdatePlayerHealth(struct sc_packet_player_health* packet);

	void ClearPlayers();
	void ClearZombies();

	void AddZombie(struct sc_packet_spawn_zombie* packet);
	void MoveZombie(struct sc_packet_zombie_move* packet);
	void AnimateZombie(struct sc_packet_zombie_state* packet);
	void DieZombie(struct sc_packet_zombie_die* packet);
	void RemoveZombieById(uint32_t zombieId);

	void ApplySnapshot(struct sc_packet_snapshot* packet);

protected:
	vector<shared_ptr<GameObject>>		_gameObjects;
	
	shared_ptr<class RenderPass>		_renderPass;

	// array<플레이어를 이루는 게임오브젝트들, 플레이어 수> _players
	// 어차피 부모는 게임오브젝트들의 0번일꺼니까
	// MovePacket으로 적용시킬때, 탐색하는 경우, _player

	vector<shared_ptr<Player>>		_localPlayer;

	unordered_map<int64, vector<shared_ptr<Player>>> _players;
	vector<vector<shared_ptr<Zombie>>>	_zombies;

	std::unordered_map<uint32_t, JumpState> _jumpStates;
};

