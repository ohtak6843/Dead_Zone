#include "pch.h"
#include "Player.h"

Player::Player()
{
	_type = GAMEOBJECT_TYPE::PLAYER;

	_name = L"Player";

	shared_ptr<PlayerInfo> info = GET_SINGLE(GameInfo)->Get<PlayerInfo>(L"Player");
	_info = *info;

	_state = PLAYER_STATE::T_POSE;
}

Player::~Player()
{
}