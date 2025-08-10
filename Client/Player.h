#pragma once
#include "GameObject.h"
#include "GameInfo.h"
#include "Gun.h"

enum class PLAYER_STATE
{
	T_POSE,
	IDLE,
	RUN_FORWARD,
	RUN_BACKWARD,
	RUN_LEFT,
	RUN_RIGHT,
	FIRE,
};

enum class PLAYER_ANIMATION_TYPE
{
	T_POSE,
	IDLE,
	RUN_FORWARD,
	RUN_BACKWARD,
	RUN_LEFT,
	RUN_RIGHT,
	FIRE,

	END,
};

class Player : public GameObject
{
public:
	Player();
	virtual ~Player();

public:
	virtual void SetState(PLAYER_STATE state) { _state = state; }
	uint32 GetHp() const { return _info.hp; }
	uint32 GetMaxHp() const { return _info.maxHp; }
	uint32 GetAttackDamage() const { return _info.attackDamage; }
	float GetWalkSpeed() const { return _info.walkSppeed; }
	float GetRunSpeed() const { return _info.runSpeed; }
	wstring GetName() const { return _info.name; }
	uint32 GetGold() const { return _info.gold; }

	void SetHp(uint32 hp) { _info.hp = hp; }
	void SetMaxHp(uint32 maxHp) { _info.maxHp = maxHp; }
	void SetAttackDamage(uint32 attackDamage) { _info.attackDamage = attackDamage; }
	void SetWalkSpeed(float walkSpeed) { _info.walkSppeed = walkSpeed; }
	void SetRunSpeed(float runSpeed) { _info.runSpeed = runSpeed; }
	void SetGold(uint32 gold) { _info.gold = gold; }

	void AddGun(vector<shared_ptr<Gun>>& guns) { _guns.push_back(guns); }
	vector<vector<shared_ptr<Gun>>>& GetGuns() { return _guns; }

protected:
	PLAYER_STATE _state;
	PlayerInfo _info;

	vector<vector<shared_ptr<Gun>>> _guns;
};

