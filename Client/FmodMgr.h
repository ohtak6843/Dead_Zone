#pragma once
#include <fmod.hpp>
#include <fmod_errors.h>

using namespace FMOD;

class FmodMgr
{
	DECLARE_SINGLE(FmodMgr);

public:
	void Init();
	void Update();

	void PlaySound();

private:
	System* _system;
	Sound* _sound[2];
	Channel* _channel[1];
};

