#include "pch.h"
#include "FmodMgr.h"

void FmodMgr::Init()
{
    FMOD_RESULT result;

    result = FMOD::System_Create(&_system);

    result = _system->init(512, FMOD_INIT_NORMAL, nullptr);

	LoadSounds();
	SetVolumeList();
}

void FmodMgr::Update()
{
	_system->update();
}

void FmodMgr::SetVolumeList()
{
	// Background Music
	_volumeList[static_cast<int32>(SOUND_TYPE::TITLE)] = 0.05f;
	_volumeList[static_cast<int32>(SOUND_TYPE::STAGE01)] = 0.05f;
	_volumeList[static_cast<int32>(SOUND_TYPE::STAGE02)] = 0.05f;

	// Player Sound Effect
	_volumeList[static_cast<int32>(SOUND_TYPE::PLAYER_RUN)] = 0.02f;
	_volumeList[static_cast<int32>(SOUND_TYPE::PLAYER_PAIN)] = 0.05f;
	_volumeList[static_cast<int32>(SOUND_TYPE::PLAYER_DIE)] = 0.2f;

	// Gun Sound Effect
	_volumeList[static_cast<int32>(SOUND_TYPE::FIRE)] = 0.1f;
	_volumeList[static_cast<int32>(SOUND_TYPE::RELOAD)] = 0.1f;

	// Zombie Sound Effect
	_volumeList[static_cast<int32>(SOUND_TYPE::ZOMBIE_ATTACK)] = 0.2f;
	_volumeList[static_cast<int32>(SOUND_TYPE::ZOMBIE_RUN)] = 0.5f;
	_volumeList[static_cast<int32>(SOUND_TYPE::ZOMBIE_PAIN)] = 0.2f;
	_volumeList[static_cast<int32>(SOUND_TYPE::ZOMBIE_DIE)] = 0.5f;

	// UI Sound Effect
	_volumeList[static_cast<int32>(SOUND_TYPE::STAGE_CLEAR)] = 0.5f;
	_volumeList[static_cast<int32>(SOUND_TYPE::GAME_OVER)] = 0.5f;
}

void FmodMgr::LoadSound(SOUND_TYPE type, const char* filePath, bool loopFlag)
{
	FMOD_RESULT result;
	
	if(loopFlag)
		result = _system->createSound(filePath, FMOD_DEFAULT | FMOD_LOOP_NORMAL, nullptr, &_soundList[static_cast<int32>(type)]);
	else
		result = _system->createSound(filePath, FMOD_DEFAULT, nullptr, &_soundList[static_cast<int32>(type)]);
	
	if (result != FMOD_OK)
	{
		return;
	}
}

void FmodMgr::LoadSounds()
{
	// Background Music
	LoadSound(SOUND_TYPE::TITLE, "..\\Resources\\Sound\\TitleScene_BGM.mp3", true);
	LoadSound(SOUND_TYPE::STAGE01, "..\\Resources\\Sound\\Stage01_BGM (Area6).mp3", true);
	LoadSound(SOUND_TYPE::STAGE02, "..\\Resources\\Sound\\Stage02_BGM (CSO_Zombie_Shelter_Night).mp3", true);

	// Player Sound Effect
	LoadSound(SOUND_TYPE::PLAYER_RUN, "..\\Resources\\Sound\\Player_Run.mp3", false);
	LoadSound(SOUND_TYPE::PLAYER_PAIN, "..\\Resources\\Sound\\Player_Pain.mp3", false);
	//LoadSound(SOUND_TYPE::PLAYER_DIE, "..\\Resources\\Sound\\Player_Die.mp3", false);

	// Gun Sound Effect
	LoadSound(SOUND_TYPE::FIRE, "..\\Resources\\Sound\\AK47_Fire.mp3", false);
	LoadSound(SOUND_TYPE::RELOAD, "..\\Resources\\Sound\\Reload.mp3", false);

	// Zombie Sound Effect
	LoadSound(SOUND_TYPE::ZOMBIE_ATTACK, "..\\Resources\\Sound\\Zombie_Attack.mp3", false);
	//LoadSound(SOUND_TYPE::ZOMBIE_RUN, "..\\Resources\\Sound\\Zombie_Run.mp3", false);
	LoadSound(SOUND_TYPE::ZOMBIE_PAIN, "..\\Resources\\Sound\\Zombie_Pain.mp3", false);
	LoadSound(SOUND_TYPE::ZOMBIE_DIE, "..\\Resources\\Sound\\Zombie_Die.mp3", false);

	// UI Sound Effect
	LoadSound(SOUND_TYPE::STAGE_CLEAR, "..\\Resources\\Sound\\Stage_Clear.mp3", false);
	LoadSound(SOUND_TYPE::GAME_OVER, "..\\Resources\\Sound\\Game_Over.mp3", false);
}

void FmodMgr::PlaySound(SOUND_TYPE type)
{
	FMOD_RESULT result;

	if (_soundList[static_cast<int32>(type)] == nullptr)
	{
		return;
	}

	result = _system->playSound(_soundList[static_cast<int32>(type)], nullptr, false, &_channelList[static_cast<int32>(type)]);

	if (result != FMOD_OK)
	{
		return;
	}

	_channelList[static_cast<int32>(type)]->setVolume(_volumeList[static_cast<int32>(type)]); // Set volume to 50% for all sounds
}

void FmodMgr::StopSound(SOUND_TYPE type)
{
	_channelList[static_cast<int32>(type)]->stop();
}

void FmodMgr::StopAllSounds()
{
	for (auto& channel : _channelList)
	{
		if (channel)
		{
			channel->stop();
		}
	}
}

bool FmodMgr::CheckPlaying(SOUND_TYPE type)
{
	FMOD_RESULT result;
	bool isPlaying = false;

	if (_channelList[static_cast<int32>(type)])
	{
		result = _channelList[static_cast<int32>(type)]->isPlaying(&isPlaying);
		if (result != FMOD_OK)
		{
			return false;
		}
	}

	return isPlaying;
}