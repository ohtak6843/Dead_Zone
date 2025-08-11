#pragma once
#include "Zombie.h"

enum class BOSS_ZOMBIE_ANIMATION
{
	ATTACK1,
	ATTACK2,
	DIE,
	IDLE,
	JUMP,
	RUN,
	SCREAM,

	END
};

class BossZombie : public Zombie
{
public:
	BossZombie(const wstring& infoKey = L"BossZombie") : Zombie(infoKey) {}
	virtual ~BossZombie() {}

	virtual void Awake() override {}
	virtual void Start() override {}
	virtual void Update() override {}
	virtual void LateUpdate() override {}

public:
	virtual void SetState(ZOMBIE_STATE playerState);
};

