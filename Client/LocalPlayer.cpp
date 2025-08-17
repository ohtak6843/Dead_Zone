#include "pch.h"
#include "LocalPlayer.h"
#include "Framework.h"
#include "Transform.h"
#include "Camera.h"
#include "GameObject.h"
#include "InputMgr.h"
#include "Timer.h"
#include "SceneMgr.h"
#include "MultiPlayer.h" 
#include "Scene.h"
#include "RaycastMgr.h"
#include "../echoserver/protocol.h"

#include "Particle.h"
#include "Gun.h"
#include "M4A1.h"
#include "AK47.h"
#include "Zombie.h"
#include "ParticleObject.h"

#include "FmodMgr.h"

LocalPlayer::LocalPlayer()
{
	_name = L"MainCamera";
}

LocalPlayer::~LocalPlayer()
{
}

void LocalPlayer::LateUpdate()
{
	/*static bool bInitialized = false;
	if (!bInitialized) {
		Vec3 spawn{ 1185.0f, 140.0f, 473.0f };
		GetTransform()->SetLocalPosition(spawn);
		bInitialized = true;
	}*/

	/*constexpr float MAP_MIN_X = 237.0f;
	constexpr float MAP_MAX_X = 2030.0f;
	constexpr float MAP_MIN_Z = -3552.0f;
	constexpr float MAP_MAX_Z = 3535.0f;
	constexpr float MAP_MIN_Y = 140.0f;
	constexpr float MAP_MAX_Y =  960.0f;*/

	constexpr float PLAYER_RADIUS = 10.0f;

	static bool  localJumping = false;
	static float localVerticalVelocity = 0.0f;
	const float  gravity = 9.8f;

	Vec3 pos = GetTransform()->GetLocalPosition();
	_moveDir = { 0.f, 0.f, 0.f };

	// 입력 처리
	ProcessKeyInput();
	ProcessMouseInput();

	static bool wasMovingLastFrame = false;
	POINT deltapos = INPUT->GetDeltaPos();
	bool isMoving = (_moveDir.x != 0.f || _moveDir.y != 0.f || _moveDir.z != 0.f);
	bool isAttacking = INPUT->GetButton(MOUSE_TYPE::LBUTTON);

	// 사운드 출력
	if (isMoving)
	{
		// 사운드 재생
		bool flag = GET_SINGLE(FmodMgr)->CheckPlaying(SOUND_TYPE::PLAYER_RUN);
		if (flag == false)
			GET_SINGLE(FmodMgr)->PlaySound(SOUND_TYPE::PLAYER_RUN);
	}
	else
	{
		GET_SINGLE(FmodMgr)->StopSound(SOUND_TYPE::PLAYER_RUN);
	}

	if (isMoving || deltapos.x != 0.f || deltapos.y != 0.f)
	{
		_moveDir.Normalize();

		pos += _moveDir * _speed * DELTA_TIME;
		GetTransform()->SetLocalPosition(pos); //서버권위 수정

		cs_packet_move pkt;
		pkt.size = sizeof(cs_packet_move);
		pkt.type = C2S_P_MOVE;
		pkt.direction.x = _moveDir.x;
		pkt.direction.y = _moveDir.y;
		pkt.direction.z = _moveDir.z;
		pkt.look.x = GetTransform()->GetLook().x;
		pkt.look.y = GetTransform()->GetLook().y;
		pkt.look.z = GetTransform()->GetLook().z;

		send(gameFramework->GetWindow().sock,
			reinterpret_cast<char*>(&pkt),
			sizeof(pkt), 0);
	}
	else if (wasMovingLastFrame)
	{
		cs_packet_move stopPkt{};
		stopPkt.size = sizeof(stopPkt);
		stopPkt.type = C2S_P_MOVE;
		stopPkt.direction.x = 0.f;
		stopPkt.direction.y = 0.f;
		stopPkt.direction.z = 0.f;
		stopPkt.look.x = GetTransform()->GetLook().x;
		stopPkt.look.y = GetTransform()->GetLook().y;
		stopPkt.look.z = GetTransform()->GetLook().z;
		
		send(gameFramework->GetWindow().sock,
			reinterpret_cast<char*>(&stopPkt),
			sizeof(stopPkt), 0);
	}
	wasMovingLastFrame = isMoving;

	if (INPUT->GetButtonDown(KEY_TYPE::SPACE) && !localJumping) {
		localJumping = true;
		const float jumpSpeed = 12.0f;
		localVerticalVelocity = jumpSpeed;
		cs_packet_jump pkt{};
		pkt.size = sizeof(cs_packet_jump);
		pkt.type = C2S_P_JUMP;
		pkt.initVelocity = jumpSpeed;
		send(gameFramework->GetWindow().sock,
			reinterpret_cast<char*>(&pkt),
			sizeof(pkt),
			0);
	}

	/*if (localJumping) {
		pos.y += localVerticalVelocity * DELTA_TIME;
		localVerticalVelocity -= gravity * DELTA_TIME;
		if (pos.y <= MAP_MIN_Y) {
			pos.y = MAP_MIN_Y;
			localJumping = false;
		}
	}

	if (pos.x - PLAYER_RADIUS < MAP_MIN_X)
		pos.x = MAP_MIN_X + PLAYER_RADIUS;
	else if (pos.x + PLAYER_RADIUS > MAP_MAX_X)
		pos.x = MAP_MAX_X - PLAYER_RADIUS;

	if (pos.z - PLAYER_RADIUS < MAP_MIN_Z)
		pos.z = MAP_MIN_Z + PLAYER_RADIUS;
	else if (pos.z + PLAYER_RADIUS > MAP_MAX_Z)
		pos.z = MAP_MAX_Z - PLAYER_RADIUS;

	if (pos.y < MAP_MIN_Y)
		pos.y = MAP_MIN_Y;*/
	// if (pos.y > MAP_MAX_Y) 
	//     pos.y = MAP_MAX_Y;

	uint8_t newState = static_cast<uint8_t>(PLAYER_STATE::IDLE);
	if (isAttacking && !isMoving) {
		newState = static_cast<uint8_t>(PLAYER_STATE::FIRE);
	}
	else if (isMoving) {
			_moveDir.Normalize();
			Vec3 forward = GetTransform()->GetLook();
			Vec3 right = GetTransform()->GetRight();
			float fwd = _moveDir.x * forward.x + _moveDir.y * forward.y + _moveDir.z * forward.z;
			float rgt = _moveDir.x * right.x + _moveDir.y * right.y + _moveDir.z * right.z;
			if (fwd > 0.5f)             newState = static_cast<uint8_t>(PLAYER_STATE::RUN_FORWARD);
			else if (fwd < -0.5f)        newState = static_cast<uint8_t>(PLAYER_STATE::RUN_BACKWARD);
			else if (rgt > 0.5f)        newState = static_cast<uint8_t>(PLAYER_STATE::RUN_RIGHT);
			else if (rgt < -0.5f)        newState = static_cast<uint8_t>(PLAYER_STATE::RUN_LEFT);
			else                         newState = static_cast<uint8_t>(PLAYER_STATE::IDLE);
	}
	else {
		newState = static_cast<uint8_t>(PLAYER_STATE::IDLE);
		}

	static uint8_t lastState = 255;
	if (newState != lastState) {
		cs_packet_state stPkt{};
		stPkt.size = sizeof(stPkt);
		stPkt.type = C2S_P_STATE;
		stPkt.state = newState;

		send(gameFramework->GetWindow().sock,
			reinterpret_cast<char*>(&stPkt),
			stPkt.size, 0);

		lastState = newState;
	}

	//GetTransform()->SetLocalPosition(pos); //서버권위
}

void LocalPlayer::ProcessKeyInput()
{
	if (INPUT->GetButtonDown(KEY_TYPE::KEY_F6))
		gameFramework->ToggleFullScreen(!FULL_SCREEN);	

	// 콜라이더 출력 여부
	if (INPUT->GetButtonDown(KEY_TYPE::KEY_F5))
		SET_DEBUG_MODE(!DEBUG_MODE);

	// 커서 출력 여부
	if (INPUT->GetButtonDown(KEY_TYPE::KEY_F4))
		INPUT->LockCursor(!INPUT->IsCursorLocked());

	if (INPUT->GetButton(KEY_TYPE::W))
	{
		_moveDir += GetTransform()->GetLook();
		_moveDir.y = 0.f;
	}
	if (INPUT->GetButton(KEY_TYPE::S))
	{
		_moveDir -= GetTransform()->GetLook();
		_moveDir.y = 0.f;
	}
	if (INPUT->GetButton(KEY_TYPE::A))
	{
		_moveDir -= GetTransform()->GetRight();
		_moveDir.y = 0.f;
	}
	if (INPUT->GetButton(KEY_TYPE::D))
	{
		_moveDir += GetTransform()->GetRight();
		_moveDir.y = 0.f;
	}

	if (INPUT->GetButton(KEY_TYPE::KEY_9))
	{
		GetTransform()->LookAt(Vec3(0.f, 0.f, -1.f));
	}

	//if (INPUT->GetButtonDown(KEY_TYPE::F))
	//{
	//	// 테스트용 임시로 대충 만듦
	//	_GunType = (_GunType + 1) % _MaxGunType;
	//	if (_GunType == 0)
	//	{
	//		GET_SINGLE(SceneMgr)->GetActiveScene()->FindGameObject(L"AK47")->SetActive(false);
	//		auto gun = GET_SINGLE(SceneMgr)->GetActiveScene()->FindGameObject(L"M4A1");
	//		gun->SetActive(true);

	//	}
	//	else if (_GunType == 1)
	//	{
	//		GET_SINGLE(SceneMgr)->GetActiveScene()->FindGameObject(L"M4A1")->SetActive(false);
	//		auto gun = GET_SINGLE(SceneMgr)->GetActiveScene()->FindGameObject(L"AK47");
	//		gun->SetActive(true);
	//	}
	//}
}

void LocalPlayer::ProcessMouseInput()
{
	wstring gunName = L"M4A1";

	switch (_GunType)
	{
	case 0:
		gunName = L"M4A1";
		break;
	case 1:
		gunName = L"AK47";
		break;
	default:
		break;
	}

	auto gun = GET_SINGLE(SceneMgr)->GetActiveScene()->FindGameObject(gunName);

	if (INPUT->GetButton(MOUSE_TYPE::LBUTTON) && _info.hp != 0.f)
	{
		bool fireSuccess = false;
		fireSuccess = static_pointer_cast<Gun>(gun)->Fire();

		if (fireSuccess)
		{
			// 사운드 출력
			GET_SINGLE(FmodMgr)->PlaySound(SOUND_TYPE::FIRE);
		}

		if (fireSuccess)
		{
			Vec3 hitPos;
			shared_ptr<Zombie> zombie;

			if (GET_SINGLE(RaycastMgr)->PickZombie(gameFramework->GetWindow().width / 2, gameFramework->GetWindow().height / 2, OUT hitPos, OUT zombie)) {
				cs_packet_attack atkPkt{};
				atkPkt.size = sizeof(atkPkt);
				atkPkt.type = C2S_P_ATTACK;
				atkPkt.zombieId = zombie ? static_cast<long long>(zombie->GetID()) : -1;
				send(gameFramework->GetWindow().sock,
					reinterpret_cast<char*>(&atkPkt),
					sizeof(atkPkt),
					0);

				if (zombie) {
					shared_ptr<ParticleObject> particleObj = zombie->GetParticle();
					if (particleObj) {
						particleObj->GetTransform()->SetLocalPosition(hitPos);
						particleObj->GetParticle()->Reset();
						particleObj->GetParticle()->SetActive(true);
					}
				}

				// 사운드 출력
				bool flag = GET_SINGLE(FmodMgr)->CheckPlaying(SOUND_TYPE::ZOMBIE_PAIN);
				if (flag == flag)
					GET_SINGLE(FmodMgr)->PlaySound(SOUND_TYPE::ZOMBIE_PAIN);
			}
		}
	}

	// 장전 버튼이 눌렸을 때
	if (INPUT->GetButtonDown(KEY_TYPE::R))
	{
		static_pointer_cast<Gun>(gun)->Reload();
	}

	// 정조준 활성화 여부
	if (INPUT->GetButton(MOUSE_TYPE::RBUTTON))
	{
		static_pointer_cast<Gun>(gun)->SetAimingFlag(true);
		GET_SINGLE(SceneMgr)->GetActiveScene()->FindGameObject(L"Crosshair")->SetActive(false); // 조준선 비활성화
	}
	else
	{
		static_pointer_cast<Gun>(gun)->SetAimingFlag(false);
		GET_SINGLE(SceneMgr)->GetActiveScene()->FindGameObject(L"Crosshair")->SetActive(true); // 조준선 활성화
	}

	POINT deltaPos = INPUT->GetDeltaPos();

	_mouseYaw += deltaPos.x * DELTA_TIME * _sensitivity;
	_mousePitch += deltaPos.y * DELTA_TIME * _sensitivity;

	// 피치 제한 (위아래)
	_mousePitch = std::clamp(_mousePitch, -89.f, 89.f);

	// 반동 감쇠
	_recoilPitch = Lerp(_recoilPitch, 0.f, 2.f * DELTA_TIME);
	_recoilYaw = Lerp(_recoilYaw, 0.f, 2.f * DELTA_TIME);

	// 최종 회전 = 마우스 + 반동
	float finalPitch = _mousePitch - _recoilPitch;
	float finalYaw = _mouseYaw - _recoilYaw;

	GetTransform()->SetLocalRotation(Vec3(finalPitch, finalYaw, 0.f));
}

void LocalPlayer::Recoil(float pitchAmount, float yawAmount)
{
	// 카메라 반동 설정
	_recoilPitch += pitchAmount;	// 수직
	_recoilYaw += yawAmount;		// 수평
}