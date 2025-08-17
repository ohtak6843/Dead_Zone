#include "pch.h"
#include "BossZombie.h"
#include "Animator.h"

#include "FmodMgr.h"

void BossZombie::SetState(ZOMBIE_STATE playerState)
{
	// 현재 상태와 패킷의 상태가 같으면 아무것도 하지 않음
	if (_state == playerState)
		return;

	_state = playerState;
	switch (playerState)
	{
	case ZOMBIE_STATE::T_POSE:
	{
		uint32 index = static_cast<uint32>(BOSS_ZOMBIE_ANIMATION::IDLE);
		GetAnimator()->Play(index);
		break;
	}
	case ZOMBIE_STATE::IDLE:
	{
		uint32 index = static_cast<uint32>(BOSS_ZOMBIE_ANIMATION::IDLE);
		GetAnimator()->Play(index);
		break;
	}
	case ZOMBIE_STATE::WALK:
	{
		uint32 index = static_cast<uint32>(BOSS_ZOMBIE_ANIMATION::RUN);
		GetAnimator()->Play(index);
		break;
	}
	case ZOMBIE_STATE::RUN:
	{
		uint32 index = static_cast<uint32>(BOSS_ZOMBIE_ANIMATION::RUN);
		GetAnimator()->Play(index);
		break;
	}
	case ZOMBIE_STATE::ATTACK:
	{
		int randomValue = rand() % 2;

		uint32 index;
		if (randomValue == 0)
			index = static_cast<uint32>(BOSS_ZOMBIE_ANIMATION::ATTACK1);
		else
			index = static_cast<uint32>(BOSS_ZOMBIE_ANIMATION::ATTACK2);

		GetAnimator()->Play(index);

		// 사운드 재생
		bool flag = GET_SINGLE(FmodMgr)->CheckPlaying(SOUND_TYPE::ZOMBIE_ATTACK);
		if (flag == false)
			GET_SINGLE(FmodMgr)->PlaySound(SOUND_TYPE::ZOMBIE_ATTACK);
		break;
	}
	case ZOMBIE_STATE::DIE:
	{
		uint32 index = static_cast<uint32>(BOSS_ZOMBIE_ANIMATION::DIE);
		GetAnimator()->Play(index);

		// 사운드 재생
		GET_SINGLE(FmodMgr)->PlaySound(SOUND_TYPE::ZOMBIE_DIE);
		break;
	}
	case ZOMBIE_STATE::JUMP:
	{
		uint32 index = static_cast<uint32>(BOSS_ZOMBIE_ANIMATION::JUMP);
		GetAnimator()->Play(index);
		break;
	}
	case ZOMBIE_STATE::SCREAM:
	{
		uint32 index = static_cast<uint32>(BOSS_ZOMBIE_ANIMATION::SCREAM);
		GetAnimator()->Play(index);

		// 사운드 재생
		GET_SINGLE(FmodMgr)->PlaySound(SOUND_TYPE::ZOMBIE_SCREAM);
		break;
	}
	default:
		break;
	}
}
