#pragma once
#include "Zombie.h"

enum class ELITE_ZOMBIE_ANIMATION
{
	ATTACK,
	DIE,
	IDLE,
	RUN,
	T_POSE,

	END
};

class EliteZombie : public Zombie
{
public:
	EliteZombie(const wstring& infoKey = L"PoliceZombie") : Zombie(infoKey) {}
	virtual ~EliteZombie() {}

	virtual void Awake() override {}
	virtual void Start() override {}
	virtual void Update() override {}
	virtual void LateUpdate() override {}
public:
	virtual void SetState(ZOMBIE_STATE);
};

