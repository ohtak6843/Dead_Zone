#include "pch.h"
#include "UICamera.h"
#include "Camera.h"

#include "Framework.h"

#include "InputMgr.h"
#include "SceneMgr.h"
#include "Scene.h"

#include "../echoserver/protocol.h"

extern std::atomic<bool>    g_augmentActive;
extern std::atomic<uint8_t> g_augmentCount;

enum class CARD_CARTEGORY
{
	PLAYER_DMG_UP
};

UICamera::UICamera()
{
}

UICamera::~UICamera()
{
}

void UICamera::LateUpdate()
{
	if (g_augmentActive) {
		SOCKET sock = gameFramework->GetWindow().sock;
		// 1번 키
		if (INPUT->GetButtonDown(KEY_TYPE::KEY_1) && g_augmentCount >= 1) {
			cs_packet_augment_select selPkt{};
			selPkt.size = sizeof(selPkt);
			selPkt.type = C2S_P_AUGMENT_SELECT;
			selPkt.selectedIndex = 0;
			send(sock, reinterpret_cast<char*>(&selPkt), selPkt.size, 0);
			g_augmentActive = false;
			GET_SINGLE(SceneMgr)->GetActiveScene()->ShowAugments(false);
		}
		// 2번 키
		else if (INPUT->GetButtonDown(KEY_TYPE::KEY_2) && g_augmentCount >= 2) {
			cs_packet_augment_select selPkt{};
			selPkt.size = sizeof(selPkt);
			selPkt.type = C2S_P_AUGMENT_SELECT;
			selPkt.selectedIndex = 1;
			send(sock, reinterpret_cast<char*>(&selPkt), selPkt.size, 0);
			GET_SINGLE(SceneMgr)->GetActiveScene()->ShowAugments(false);
		}
		// 3번 키
		else if (INPUT->GetButtonDown(KEY_TYPE::KEY_3) && g_augmentCount >= 3) {
			cs_packet_augment_select selPkt{};
			selPkt.size = sizeof(selPkt);
			selPkt.type = C2S_P_AUGMENT_SELECT;
			selPkt.selectedIndex = 2;
			send(sock, reinterpret_cast<char*>(&selPkt), selPkt.size, 0);
			GET_SINGLE(SceneMgr)->GetActiveScene()->ShowAugments(false);
		}
	}
}
