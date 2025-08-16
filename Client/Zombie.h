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
	JUMP,
	SCREAM,

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
	shared_ptr<class ParticleObject> GetParticle() { return _particle; }
	//shared_ptr<class ParticleObject> FindInactiveParticle();

protected:
	ZOMBIE_STATE _state;
	ZombieInfo _info;
	shared_ptr<class ParticleObject> _particle;
	shared_ptr<class BloodParticle> _blood;

private:
	bool _initialized = false;
	float _changeDirectionTime;
	float _elapsedTime;
	bool _moving;
	float _pauseDuration{};

	bool _start;
};

