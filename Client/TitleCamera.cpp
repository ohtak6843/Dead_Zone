#include "pch.h"
#include "TitleCamera.h"
#include "InputMgr.h"
#include "SceneMgr.h"
#include "RaycastMgr.h"
#include "Framework.h"

#include "../echoserver/protocol.h"

TitleCamera::TitleCamera()
{
}

TitleCamera::~TitleCamera()
{
}

void TitleCamera::LateUpdate()
{
	//if (INPUT->GetButtonDown(MOUSE_TYPE::LBUTTON))
	//{
	//	shared_ptr<GameObject> obj = GET_SINGLE(RaycastMgr)->Pick(gameFramework->GetWindow().width / 2, gameFramework->GetWindow().height / 2);

	//	if (obj)
	//	{
	//		if (obj->GetName() == L"GameStartButton")
	//		{
	//			GET_SINGLE(SceneMgr)->SetChangeScene(true);
	//			GET_SINGLE(SceneMgr)->SetNextSceneType(SCENE_TYPE::STAGE01);
	//		}
	//	}
	//}

	//if (INPUT->GetButtonDown(KEY_TYPE::SPACE))
	//{
	//	GET_SINGLE(SceneMgr)->SetChangeScene(true);
	//	GET_SINGLE(SceneMgr)->SetNextSceneType(SCENE_TYPE::STAGE01);
	//}

	//constexpr float left = (WINDOW_WIDTH / 2) + 380.f - 400.f; // 620
	//constexpr float right = (WINDOW_WIDTH / 2) + 380.f + 400.f; // 1420
	//constexpr float top = (WINDOW_HEIGHT / 2) + 280.f - 200.f; // 480
	//constexpr float bottom = (WINDOW_HEIGHT / 2) + 280.f + 200.f; // 880

	constexpr float left = 810.f;
	constexpr float right = 1210.f;
	constexpr float top = 550.f;
	constexpr float bottom = 750.f;

	if (INPUT->GetButtonUp(MOUSE_TYPE::LBUTTON))
	{
		POINT mousePos = INPUT->GetMousePos();

		if (mousePos.x >= left && mousePos.x <= right &&
			mousePos.y <= bottom && mousePos.y >= top)
		{
			// 자동 로그인: cs_packet_login 패킷 전송
			cs_packet_login loginPacket;
			loginPacket.size = sizeof(cs_packet_login);
			loginPacket.type = C2S_P_LOGIN;
			int sendResult = send(gameFramework->GetWindow().sock, reinterpret_cast<char*>(&loginPacket), sizeof(loginPacket), 0);
			if (sendResult == SOCKET_ERROR) {
				closesocket(gameFramework->GetWindow().sock);
				WSACleanup();
			}

			GET_SINGLE(SceneMgr)->SetChangeScene(true);
			GET_SINGLE(SceneMgr)->SetNextSceneType(SCENE_TYPE::STAGE01);
		}
	}
}
