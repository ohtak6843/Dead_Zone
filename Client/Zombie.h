#pragma once
#include "GameObject.h"
#include "GameInfo.h"

enum class ZOMBIE_STATE
{
	T_POSE,
	IDLE,
	WALK,
	RUN,
	ATTACK,
	DIE,

	END
};

enum class NORMAL_ZOMBIE_ANIMATION
{
	ATTACK,
	DIE,
	IDLE,
	RUN,

	END
};

class Zombie : public GameObject
{
public:
	Zombie(const wstring& infoKey = L"NormalZombie");
	virtual ~Zombie();

	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;
	virtual void LateUpdate() override;

public:
	virtual void SetState(ZOMBIE_STATE);

public:
	void SetRandomDirection();
	void SetPauseDuration();
	void Move();
	array<shared_ptr<class ParticleObject>, 20> GetParticles() { return _particles; }
	shared_ptr<class ParticleObject> FindInactiveParticle();

protected:
	ZOMBIE_STATE _state;
	ZombieInfo _info;
	array<shared_ptr<class ParticleObject>, 20> _particles;
	array<shared_ptr<class BloodParticle>, 20> _blood;

private:
	bool _initialized = false;
	float _changeDirectionTime;
	float _elapsedTime;
	bool _moving;
	float _pauseDuration{};

	bool _start;
};

