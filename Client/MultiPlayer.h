#pragma once
#include "Player.h"

class MultiPlayer : public Player
{
public:
	MultiPlayer();
	virtual ~MultiPlayer();

public:
	virtual void SetState(PLAYER_STATE playerState) override;
};