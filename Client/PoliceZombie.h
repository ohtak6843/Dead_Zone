#pragma once
#include "Zombie.h"


enum class POLICE_ZOMBIE_ANIMATION
{
	ATTACK,
	DIE,
	IDLE,
	RUN,
	T_POSE,

	END
};

class PoliceZombie : public Zombie
{
public:
	PoliceZombie(const wstring& infoKey = L"PoliceZombie") : Zombie(infoKey) {}
	virtual ~PoliceZombie() {}

	virtual void Awake() override {}
	virtual void Start() override {}
	virtual void Update() override {}
	virtual void LateUpdate() override {}
public:
	virtual void SetState(ZOMBIE_STATE);
};

