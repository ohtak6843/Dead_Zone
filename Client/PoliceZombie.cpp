#include "pch.h"
#include "PoliceZombie.h"
#include "Animator.h"

void PoliceZombie::SetState(ZOMBIE_STATE playerState)
{
	// 현재 상태와 패킷의 상태가 같으면 아무것도 하지 않음
	if (_state == playerState)
		return;

	_state = playerState;
	switch (playerState)
	{
	case ZOMBIE_STATE::T_POSE:
	{
		uint32 index = static_cast<uint32>(POLICE_ZOMBIE_ANIMATION::IDLE);
		GetAnimator()->Play(index);
		break;
	}
	case ZOMBIE_STATE::IDLE:
	{
		uint32 index = static_cast<uint32>(POLICE_ZOMBIE_ANIMATION::IDLE);
		GetAnimator()->Play(index);
		break;
	}
	case ZOMBIE_STATE::WALK:
	{
		uint32 index = static_cast<uint32>(POLICE_ZOMBIE_ANIMATION::RUN);
		GetAnimator()->Play(index);
		break;
	}
	case ZOMBIE_STATE::RUN:
	{
		uint32 index = static_cast<uint32>(POLICE_ZOMBIE_ANIMATION::RUN);
		GetAnimator()->Play(index);
		break;
	}
	case ZOMBIE_STATE::ATTACK:
	{
		uint32 index = static_cast<uint32>(POLICE_ZOMBIE_ANIMATION::ATTACK);
		GetAnimator()->Play(index);
		break;
	}
	case ZOMBIE_STATE::DIE:
	{
		uint32 index = static_cast<uint32>(POLICE_ZOMBIE_ANIMATION::DIE);
		GetAnimator()->Play(index);
		break;
	}
	default:
		break;
	}
}
