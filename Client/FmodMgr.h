#pragma once
#include <fmod.hpp>
#include <fmod_errors.h>

using namespace FMOD;

enum class SOUND_TYPE
{
	// Background Music
	TITLE,
	STAGE01,
	STAGE02,

	// Player Sound Effect
	PLAYER_RUN,
	PLAYER_PAIN,
	PLAYER_DIE,

	// Gun Sound Effect
	FIRE,
	RELOAD,

	// Zombie Sound Effect
	ZOMBIE_ATTACK,
	ZOMBIE_RUN,
	ZOMBIE_PAIN,
	ZOMBIE_DIE,

	// UI Sound Effect
	STAGE_CLEAR,
	GAME_OVER,
	END
};

class FmodMgr
{
	DECLARE_SINGLE(FmodMgr);

public:
	void Init();
	void Update();
	
	void SetVolumeList();

	void LoadSound(SOUND_TYPE type, const char* filePath, bool loopFlag);
	void LoadSounds();

	void PlaySound(SOUND_TYPE type);
	
	void StopSound(SOUND_TYPE type);
	void StopAllSounds();

	bool CheckPlaying(SOUND_TYPE type);

private:
	System* _system;

	array<Sound*, static_cast<int32>(SOUND_TYPE::END)> _soundList;
	array<Channel*, static_cast<int32>(SOUND_TYPE::END)> _channelList;

	array<float, static_cast<int32>(SOUND_TYPE::END)> _volumeList;
};

