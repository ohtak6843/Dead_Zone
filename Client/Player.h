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
	wstring GetName() const { return _info.name; }

	void SetHp(uint32 hp) { _info.hp = hp; }

	void AddGun(vector<shared_ptr<Gun>>& guns) { _guns.push_back(guns); }

protected:
	PLAYER_STATE _state;
	PlayerInfo _info;

	vector<vector<shared_ptr<Gun>>> _guns;
};

