#include "pch.h"
#include "FmodMgr.h"

void FmodMgr::Init()
{
    FMOD_RESULT result;

    result = FMOD::System_Create(&_system);

    result = _system->init(512, FMOD_INIT_NORMAL, nullptr);

	result = _system->createSound("..\\Resources\\Sound\\background.mp3", FMOD_DEFAULT, nullptr, &_sound[0]);
}

void FmodMgr::Update()
{
	_system->update();
}

void FmodMgr::PlaySound(SOUND_TYPE type)
{
	//_system->playSound(_sound[0], nullptr, false, &_channel[0]);

	//_channel[0]->setVolume(0.05f); // º¼·ý ¼³Á¤
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
