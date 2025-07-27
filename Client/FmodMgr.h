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
	PLAYER_DEATH,

	// Zombie Sound Effect
	ZOMBIE_RUN,
	ZOMBIE_PAIN,
	ZOMBIE_DEATH,

	// UI Sound Effect
	STAGE_CLEAR,
	END
};

class FmodMgr
{
	DECLARE_SINGLE(FmodMgr);

public:
	void Init();
	void Update();
	
	void LoadSound(SOUND_TYPE type, const char* filePath);

	void PlaySound(SOUND_TYPE type);
	
	void StopSound(SOUND_TYPE type);
	void StopAllSounds();

private:
	System* _system;
	Sound* _sound[2];
	Channel* _channel[1];

	array<Sound*, static_cast<int32>(SOUND_TYPE::END)> _soundList;
	array<Channel*, static_cast<int32>(SOUND_TYPE::END)> _channelList;
};

