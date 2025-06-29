#include "pch.h"
#include "Zombie.h"
#include "Timer.h"
#include "InputMgr.h"
#include "Transform.h"
#include "BaseCollider.h"
#include "Animator.h"

#include "Scene.h"
#include "SceneMgr.h"
#include "Pch.h"

#include "BloodParticle.h"
#include "GameObject.h"
#include "ParticleObject.h"

Zombie::Zombie()
{
	_type = GAMEOBJECT_TYPE::ZOMBIE;
	_name = L"Zombie";

	shared_ptr<ZombieInfo> info = GET_SINGLE(GameInfo)->Get<ZombieInfo>(L"NormalZombie");
	_info = *info;

	//SetRandomDirection();
	//SetPauseDuration();

	_moving = false;
	_elapsedTime = 0.0f;


	// 파티클 생성
	_particle = make_shared<ParticleObject>();
	_particle->SetTransform(make_shared<Transform>());
	_blood = make_shared<BloodParticle>();
	_particle->SetParticle(_blood);


	//_particle->GetTransform()->SetParent(GetTransform());
	//_particle->GetTransform()->SetLocalPosition(Vec3(0.f, 13.f, 100.f));
	_particle->SetCheckFrustum(false);

	GET_SINGLE(SceneMgr)->GetActiveScene()->AddGameObject(_particle);
	_blood->SetActive(false);
}

Zombie::~Zombie()
{
}

void Zombie::Awake()
{
}

void Zombie::Start()
{

}

void Zombie::Update()
{

}

void Zombie::LateUpdate()
{

}

void Zombie::SetState(ZOMBIE_STATE playerState)
{
	// 현재 상태와 패킷의 상태가 같으면 아무것도 하지 않음
	if (_state == playerState)
		return;

	_state = playerState;
	switch (playerState)
	{
	case ZOMBIE_STATE::T_POSE:
	{
		uint32 index = static_cast<uint32>(NORMAL_ZOMBIE_ANIMATION::T_POSE);
		GetAnimator()->Play(index);
		break;
	}
	case ZOMBIE_STATE::IDLE:
	{
		uint32 index = static_cast<uint32>(NORMAL_ZOMBIE_ANIMATION::IDLE1);
		GetAnimator()->Play(index);
		break;
	}
	case ZOMBIE_STATE::WALK:
	{
		uint32 index = static_cast<uint32>(NORMAL_ZOMBIE_ANIMATION::RUN);
		GetAnimator()->Play(index);
		break;
	}
	case ZOMBIE_STATE::RUN:
	{
		uint32 index = static_cast<uint32>(NORMAL_ZOMBIE_ANIMATION::RUN);
		GetAnimator()->Play(index);
		break;
	}
	case ZOMBIE_STATE::ATTACK:
	{
		uint32 index = static_cast<uint32>(NORMAL_ZOMBIE_ANIMATION::ATTACK1);
		GetAnimator()->Play(index);
		break;
	}
	case ZOMBIE_STATE::DIE:
	{
		uint32 index = static_cast<uint32>(NORMAL_ZOMBIE_ANIMATION::DIE1);
		GetAnimator()->Play(index);
		break;
	}
	default:
		break;
	}
}


void Zombie::SetRandomDirection()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> angleDis(0.f, 360.f);

	float angle = angleDis(gen) * (3.141592f / 180.0f);
	Vec3 direction;
	direction.x = cos(angle);
	direction.z = sin(angle);
	direction.y = 0.0f;
}

void Zombie::SetPauseDuration()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> pauseDis(1.f, 5.f);
	_pauseDuration = pauseDis(gen);
}

void Zombie::Move()
{
	if (DELTA_TIME > 1.f)
		return;
	Vec3 pos = GetTransform()->GetLocalPosition();
	Vec3 direction = { 0.f, 0.f, -1.f };
	pos += direction * _info.walkSpeed * DELTA_TIME;
	GetTransform()->SetLocalPosition(pos);
}
