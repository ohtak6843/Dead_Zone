#pragma once
#include "GameObject.h"
#include "GameInfo.h"

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
	
protected:
	PLAYER_STATE _state;
	PlayerInfo _info;
};

