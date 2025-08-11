#include "server.h"
#include "GameRoom.h"
#include "GameManager.h"
#include "Zombie.h"
//#include "db_authentication.h"
#include "protocol.h"

#include <winsock2.h>
#include <windows.h>
#include <mswsock.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <cstring>
#include <algorithm>
#include "MapColliderLoader.h"

#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_PORT      9000
#define NUM_WORKER_THREADS 4
#define NUM_POST_ACCEPTS   10

SOCKET g_listenSocket = INVALID_SOCKET;
HANDLE  g_hIOCP = NULL;
std::mutex g_lobbyMutex;
std::queue<PER_SOCKET_CONTEXT*> g_lobbyQueue;
std::mutex g_playersMutex;
std::vector<PER_SOCKET_CONTEXT*> g_connectedPlayers;

std::atomic<bool> g_running{ true };

LPFN_ACCEPTEX lpfnAcceptEx = NULL;

std::vector<Collider> mapColliders;

GameRoom* FindGameRoomForPlayer(PER_SOCKET_CONTEXT* player) {
    for (auto* room : activeRooms) {
        for (auto* p : room->players) {
            if (p == player)
                return room;
        }
    }
    return nullptr;
}

void PostAccept(SOCKET listenSocket);
void PostRecv(PER_SOCKET_CONTEXT* pContext, PER_IO_DATA* pIoData);
void PostSendPacket(PER_SOCKET_CONTEXT* pContext, const void* packet, size_t packetSize);
void ProcessClientMessage(PER_SOCKET_CONTEXT* pContext, PER_IO_DATA* pIoData, int bytesTransferred);
void MatchmakingCheck();
void WorkerThread(HANDLE hIOCP);

void PostSendPacket(PER_SOCKET_CONTEXT* pContext, const void* packet, size_t packetSize) {
    {
        auto buf = reinterpret_cast<const unsigned char*>(packet);
        unsigned char pktSize = buf[0];
        unsigned char pktType = buf[1];
        printf("[Send] socket=%d  Size=%u  Type=%u\n",
            pContext->socket,
            pktSize,
            pktType);
        }
    PER_IO_DATA* pIoData = new PER_IO_DATA;
    memcpy(pIoData->buffer, packet, packetSize);
    pIoData->wsabuf.buf = pIoData->buffer;
    pIoData->wsabuf.len = static_cast<ULONG>(packetSize);
    pIoData->operationType = IO_WRITE;
    ZeroMemory(&pIoData->overlapped, sizeof(OVERLAPPED));

    DWORD bytesSent = 0;
    int result = WSASend(pContext->socket, &pIoData->wsabuf, 1, &bytesSent, 0, &pIoData->overlapped, NULL);
    if (result == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err != WSA_IO_PENDING) {
            printf("WSASend error: %d\n", err);
            delete pIoData;
        }
    }
}

void PostSend(PER_SOCKET_CONTEXT* pContext, const std::string& msg, PER_IO_DATA* pIoData) {
    strcpy_s(pIoData->buffer, MAX_BUFFER, msg.c_str());
    pIoData->wsabuf.buf = pIoData->buffer;
    pIoData->wsabuf.len = (ULONG)msg.length();
    pIoData->operationType = IO_WRITE;
    ZeroMemory(&pIoData->overlapped, sizeof(OVERLAPPED));

    DWORD bytesSent = 0;
    int result = WSASend(pContext->socket, &pIoData->wsabuf, 1, &bytesSent, 0, &pIoData->overlapped, NULL);
    if (result == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err != WSA_IO_PENDING) {
            printf("WSASend 에러: %d\n", err);
            delete pIoData;
        }
    }
}

void PostAccept(SOCKET listenSocket) {
    PER_IO_DATA* pAcceptIoData = new PER_IO_DATA;
    pAcceptIoData->operationType = IO_ACCEPT;
    pAcceptIoData->acceptSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (pAcceptIoData->acceptSocket == INVALID_SOCKET) {
        printf("새 accept 소켓 생성 실패: %d\n", WSAGetLastError());
        delete pAcceptIoData;
        return;
    }
    ZeroMemory(&pAcceptIoData->overlapped, sizeof(OVERLAPPED));

    DWORD bytesReceived = 0;
    if (lpfnAcceptEx(listenSocket, pAcceptIoData->acceptSocket, pAcceptIoData->buffer, 0,
        sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
        &bytesReceived, &pAcceptIoData->overlapped) == FALSE) {
        int err = WSAGetLastError();
        if (err != ERROR_IO_PENDING) {
            printf("AcceptEx 호출 실패: %d\n", err);
            closesocket(pAcceptIoData->acceptSocket);
            delete pAcceptIoData;
        }
    }
}

void PostRecv(PER_SOCKET_CONTEXT* pContext, PER_IO_DATA* pIoData) {
    pIoData->wsabuf.buf = pIoData->buffer;
    pIoData->wsabuf.len = MAX_BUFFER;
    pIoData->operationType = IO_READ;
    ZeroMemory(&pIoData->overlapped, sizeof(OVERLAPPED));

    DWORD flags = 0, bytesRecv = 0;
    int result = WSARecv(pContext->socket, &pIoData->wsabuf, 1, &bytesRecv, &flags, &pIoData->overlapped, NULL);
    if (result == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err != WSA_IO_PENDING) {
            printf("WSARecv 에러: %d\n", err);
            delete pIoData;
        }
    }
}

void WorkerThread(HANDLE) {
    DWORD bytesTransferred;
    ULONG_PTR completionKey;
    LPOVERLAPPED pOverlapped;

    while (true) {
        BOOL result = GetQueuedCompletionStatus(
            g_hIOCP, &bytesTransferred, &completionKey, &pOverlapped, INFINITE);
        DWORD lastErr = result ? 0 : GetLastError();
            if (!result && pOverlapped != nullptr &&
                (lastErr == ERROR_NETNAME_DELETED || lastErr == ERROR_OPERATION_ABORTED)) {
                bytesTransferred = 0;
            
        }
         else if (!result) {
            printf("GetQueuedCompletionStatus 에러: %d\n", lastErr);
            continue;
            
        }
        if (pOverlapped == NULL)  
            break;

        auto* pIoData = (PER_IO_DATA*)pOverlapped;
        auto* pContext = (PER_SOCKET_CONTEXT*)completionKey;

        if (pIoData->operationType == IO_ACCEPT) {
            
            SOCKET acceptedSocket = pIoData->acceptSocket;
            if (setsockopt(acceptedSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
                (char*)&g_listenSocket, sizeof(g_listenSocket)) == SOCKET_ERROR) {
                printf("SO_UPDATE_ACCEPT_CONTEXT 실패: %d\n", WSAGetLastError());
                closesocket(acceptedSocket);
                delete pIoData;
                continue;
            }

            PER_SOCKET_CONTEXT* newContext = new PER_SOCKET_CONTEXT;
            newContext->socket = acceptedSocket;
            newContext->state = STATE_LOGIN;
            newContext->username.clear();
            newContext->health = 100;
            newContext->maxHealth = 100;
            newContext->gold = 0;
            newContext->posX = newContext->posY = newContext->posZ = 0.0f;
            newContext->look = { 0.0f, 0.0f, 1.0f };
            newContext->walkSpeed = 300.0f;
            newContext->moveX = newContext->moveY = newContext->moveZ = 0.0f;
            newContext->isJumping = true;
            newContext->verticalVelocity = 0.0f;
            newContext->damage = 20;

            CreateIoCompletionPort((HANDLE)acceptedSocket, g_hIOCP, (ULONG_PTR)newContext, 0);
            PER_IO_DATA* pRecvIoData = new PER_IO_DATA;
            pRecvIoData->operationType = IO_READ;
            pRecvIoData->wsabuf.buf = pRecvIoData->buffer;
            pRecvIoData->wsabuf.len = MAX_BUFFER;
            ZeroMemory(&pRecvIoData->overlapped, sizeof(OVERLAPPED));
            PostRecv(newContext, pRecvIoData);

            {
                std::lock_guard<std::mutex> lock(g_playersMutex);
                g_connectedPlayers.push_back(newContext);
            }
            printf("새 연결 수락: socket %d\n", acceptedSocket);

            PostAccept(g_listenSocket);
            delete pIoData;
        }
        else if (pIoData->operationType == IO_READ) {
            if (bytesTransferred == 0) {
                printf("클라이언트 종료: socket %d\n", pContext->socket);

                if (auto* room = FindGameRoomForPlayer(pContext)) {
                    auto& vec = room->players;
                    vec.erase(std::remove(vec.begin(), vec.end(), pContext),
                        vec.end());
                    
                    if (vec.empty()) {
                        delete room;  
                    }
                }

                sc_packet_player_leave leavePacket{};
                leavePacket.size = sizeof(sc_packet_player_leave);
                leavePacket.type = S2C_P_PLAYER_LEAVE;
                leavePacket.playerId = pContext->socket;
                if (auto* room = FindGameRoomForPlayer(pContext)) {
                    for (auto* peer : room->players)
                        PostSendPacket(peer, &leavePacket, leavePacket.size);
                }

                {
                    std::lock_guard<std::mutex> lock(g_playersMutex);
                    g_connectedPlayers.erase(
                        std::remove(g_connectedPlayers.begin(),
                            g_connectedPlayers.end(),
                            pContext),
                        g_connectedPlayers.end());
                }
                closesocket(pContext->socket);
                delete pContext;
                delete pIoData;
                continue;
            }
            ProcessClientMessage(pContext, pIoData, bytesTransferred);

            ZeroMemory(&pIoData->overlapped, sizeof(OVERLAPPED));
            PostRecv(pContext, pIoData);
        }
    }
}

void ProcessClientMessage(PER_SOCKET_CONTEXT* pContext,
    PER_IO_DATA* pIoData,
    int                 bytesTransferred)
{
    if (bytesTransferred < 2) return;
    unsigned char packetSize = pIoData->buffer[0];
    char          packetType = pIoData->buffer[1];
    printf("socket %d → Size=%d, Type=%d\n", pContext->socket, packetSize, packetType);

    switch (packetType) {
    case C2S_P_LOGIN: {
        pContext->username = "Player_" + std::to_string(pContext->socket);
        pContext->state = STATE_LOBBY;
        pContext->health = 100;
        pContext->maxHealth = 100;
        pContext->posX = 1185.0f;
        pContext->posY = 0.0f;
        pContext->posZ = 473.0f;
        pContext->look = { 0.0f, 0.0f, 1.0f };
        pContext->walkSpeed = 300.0f;
        pContext->moveX = 0.0f;
        pContext->moveY = 0.0f;
        pContext->moveZ = 0.0f;
        pContext->isJumping = true;
        pContext->verticalVelocity = 0.0f;
        pContext->damage = 20;
        pContext->gold = 0;

        sc_packet_login_ok loginOk;
        loginOk.size = sizeof(sc_packet_login_ok);
        loginOk.type = S2C_P_LOGIN_OK;
        loginOk.playerId = pContext->socket;
        loginOk.position = { pContext->posX, pContext->posY, pContext->posZ };
        loginOk.health = pContext->health;
        loginOk.walkSpeed = pContext->walkSpeed;
        loginOk.damage = pContext->damage;
        loginOk.maxHealth = pContext->maxHealth;
        loginOk.gold = pContext->gold;

        PostSendPacket(pContext, &loginOk, loginOk.size);
        break;
    }

    case C2S_P_MOVE: {
        if (bytesTransferred < sizeof(cs_packet_move)) break;
        auto* pkt = reinterpret_cast<cs_packet_move*>(pIoData->buffer);
        Vector3 dir = pkt->direction;

        pContext->moveX = dir.x;
        pContext->moveY = dir.y;
        pContext->moveZ = dir.z;
        pContext->look = pkt->look;
       
        sc_packet_move ev{};
        ev.size = sizeof(ev);
        ev.type = S2C_P_MOVE;
        ev.playerId = pContext->socket;
        ev.position = { pContext->posX, pContext->posY, pContext->posZ };
        ev.look = pContext->look;
        if (auto* room = FindGameRoomForPlayer(pContext)) {
            for (auto* peer : room->players) {
                if (peer == pContext) {
                    // 자기 자신에게 보낼 때만 Y를 +90
                    sc_packet_move evSelf = ev;
                    evSelf.position.y += 140.0f;
                    PostSendPacket(peer, &evSelf, evSelf.size);
                }
                else {
                    // 다른 플레이어들에겐 원본(ev) 그대로
                    PostSendPacket(peer, &ev, ev.size);
                }
            }
        }
        break;
    }

    case C2S_P_ATTACK: {
        if (bytesTransferred < sizeof(cs_packet_attack)) break;
        auto* pkt = reinterpret_cast<cs_packet_attack*>(pIoData->buffer);
        long long zid = pkt->zombieId;

        if (auto* room = FindGameRoomForPlayer(pContext)) {
            auto it = std::find_if(
                room->zombies.begin(), room->zombies.end(),
                [zid](const Zombie& z) { return z.id == zid; }
            );
            if (it == room->zombies.end()) break;

            it->health -= pContext->damage;

            if (it->health <= 0) {
                sc_packet_zombie_die diePkt{};
                diePkt.size = static_cast<unsigned char>(sizeof(diePkt));
                diePkt.type = S2C_P_ZOMBIE_DIE;
                diePkt.zombieId = zid;
                for (auto* peer : room->players)
                    PostSendPacket(peer, &diePkt, diePkt.size);

                room->zombies.erase(it);
                room->killCount++;

                const int killThreshold = 1;   
                const int maxStage = 3;       

                if (room->killCount >= killThreshold && room->currentStage < maxStage) {
                    room->killCount = 0;
                    room->nextStage = room->currentStage + 1;  
                    for (const auto& z : room->zombies) {
                        sc_packet_zombie_die diePkt{};
                        diePkt.size = sizeof(diePkt);
                        diePkt.type = S2C_P_ZOMBIE_DIE;
                        diePkt.zombieId = z.id;
                        for (auto* peer : room->players) PostSendPacket(peer, &diePkt, diePkt.size);
                    }
                    room->zombies.clear();
                    room->stageChangeTimer = 10.0f;
                    room->spawnPaused = true;

                    sc_packet_stage_clear stagePkt{};
                    stagePkt.size = sizeof(stagePkt);
                    stagePkt.type = S2C_P_STAGE_CLEAR;
                    for (auto* peer : room->players)
                        PostSendPacket(peer, &stagePkt, stagePkt.size);

                    room->SendAugmentOptions();  // 원하면 유지/삭제
                }
                else if (room->currentStage == maxStage && room->killCount >= 1 /*보스 처치 조건*/) {
                    room->zombies.clear();
                    room->spawnPaused = true;
                    room->killCount = 0;
                    room->gameClearTimer = 10.0f;

                    sc_packet_game_clear clearPkt{};
                    clearPkt.size = sizeof(clearPkt);
                    clearPkt.type = S2C_P_GAME_CLEAR;
                    for (auto* peer : room->players)
                        PostSendPacket(peer, &clearPkt, clearPkt.size);
                }
            }
        }
        break;
    }

    case C2S_P_JUMP: {
        if (auto* room = FindGameRoomForPlayer(pContext)) {            
                    room->killCount = 0;
                    room->nextStage = room->currentStage + 1;
                    room->stageChangeTimer = 10.0f;
                    room->zombies.clear();
                    room->spawnPaused = true;
                    sc_packet_stage_clear stagePkt{};
                    stagePkt.size = sizeof(stagePkt);
                    stagePkt.type = S2C_P_STAGE_CLEAR;
                    for (auto* peer : room->players)
                        PostSendPacket(peer, &stagePkt, stagePkt.size);
                             
        }
        break;
    }
    case C2S_P_STATE: {
        if (bytesTransferred < sizeof(cs_packet_state))
            break;

        auto* req = reinterpret_cast<cs_packet_state*>(pIoData->buffer);

        sc_packet_state ev{};
        ev.size = sizeof(ev);
        ev.type = S2C_P_STATE;
        ev.playerId = pContext->socket;
        ev.state = req->state;

        if (auto* room = FindGameRoomForPlayer(pContext)) {
            for (auto* peer : room->players) 
                if (peer != pContext)
                    PostSendPacket(peer, &ev, ev.size);
        }
        break;
        }
    case C2S_P_SCENE_LOADED: {
        {
            std::lock_guard<std::mutex> lock(g_lobbyMutex);
            g_lobbyQueue.push(pContext);
        }
        MatchmakingCheck();
        break;
    }
    case C2S_P_STAGE_LOADED:  
        if (auto* room = FindGameRoomForPlayer(pContext)) {
            room->zombies.clear();
            room->spawnPaused = (room->currentStage == 3);
            room->killCount = 0;
            room->stageReadyCount++;
            
            if (room->stageReadyCount == (int)room->players.size()) {
                sc_packet_game_start gs{};
                gs.size = sizeof(gs);
                gs.type = S2C_P_GAME_START;  
                for (auto* pl : room->players)
                    PostSendPacket(pl, &gs, gs.size);

                for (auto* pl : room->players) {
                    sc_packet_player_info info{};
                    info.size = sizeof(info);
                    info.type = S2C_P_PLAYER_INFO;
                    info.playerId = pl->socket;
                    info.position = { pl->posX, pl->posY, pl->posZ };
                    info.health = pl->health;
                    info.walkSpeed = pl->walkSpeed;
                    info.maxHealth = pl->maxHealth;
                    info.gold = pl->gold;
                    info.damage = pl->damage;

                    for (auto* peer : room->players)
                        PostSendPacket(peer, &info, info.size);
                }
                if (room->currentStage == 3) {
                    room->QueueStartBossPhase(room->bossSpawnPos);
                }
            }
            break;
        }
    case C2S_P_AUGMENT_SELECT: {
        auto* pkt = reinterpret_cast<cs_packet_augment_select*>(pIoData->buffer);
        uint8_t idx = pkt->selectedIndex;
        if (auto* room = FindGameRoomForPlayer(pContext)) {
            room->HandleAugmentSelect(pContext, idx);
        }
        break;
    }
    default: {
        printf("정의되지 않은 패킷 타입: %d\n", packetType);
        break;
    }
    }
}

constexpr size_t kMaxPlayers = 1;

void MatchmakingCheck() {
    std::lock_guard<std::mutex> lock(g_lobbyMutex);

    while (g_lobbyQueue.size() >= kMaxPlayers) {
        std::vector<PER_SOCKET_CONTEXT*> players;
        players.reserve(kMaxPlayers);
        for (size_t i = 0; i < kMaxPlayers; ++i) {
            players.push_back(g_lobbyQueue.front());
            g_lobbyQueue.pop();
        }

        for (auto* pl : players) {
            pl->state = STATE_GAME;
        }

        sc_packet_game_start gameStart{};
        gameStart.size = sizeof(gameStart);
        gameStart.type = S2C_P_GAME_START;
        {
            std::lock_guard<std::mutex> lock2(g_playersMutex);
            for (auto* pl : players) {
                PostSendPacket(pl, &gameStart, gameStart.size);
            }
        }

        sc_packet_player_info info{};
        info.size = sizeof(info);
        info.type = S2C_P_PLAYER_INFO;
        for (auto* pl : players) {
            info.playerId = pl->socket;
            info.position = { pl->posX, pl->posY, pl->posZ };
            info.health = pl->health;
            info.walkSpeed = pl->walkSpeed;
            info.maxHealth = pl->maxHealth;
            info.gold = pl->gold;
            info.damage = pl->damage;
            for (auto* peer : players) {
                PostSendPacket(peer, &info, info.size);
            }
        }

        new GameRoom(players);

        std::ostringstream oss;
        oss << "게임룸 생성:";
        for (auto* pl : players) {
            oss << " " << pl->username;
        }
        printf("%s\n", oss.str().c_str());
    }
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    g_listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP,
        NULL, 0, WSA_FLAG_OVERLAPPED);
    if (g_listenSocket == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        WSACleanup();
        return 1;
    }

    SOCKADDR_IN sa = {};
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port = htons(DEFAULT_PORT);
    if (bind(g_listenSocket, (SOCKADDR*)&sa, sizeof(sa)) == SOCKET_ERROR ||
        listen(g_listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("bind/listen failed\n");
        closesocket(g_listenSocket);
        WSACleanup();
        return 1;
    }
    printf("Listening on port %d\n", DEFAULT_PORT);

    g_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    CreateIoCompletionPort((HANDLE)g_listenSocket, g_hIOCP, 0, 0);

    try {
        mapColliders = MapColliderLoader::Load("../Resources/json/Stage01_Collider.json");

    }
    catch (const std::exception& e) {
        std::cerr << "맵 콜라이더 로드 실패: " << e.what() << std::endl;
        return -1;
    }
    std::cout << "Loaded colliders: " << mapColliders.size() << "\n";

    GUID guidAcceptEx = WSAID_ACCEPTEX;
    DWORD bytes = 0;
    WSAIoctl(g_listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
        &guidAcceptEx, sizeof(guidAcceptEx),
        &lpfnAcceptEx, sizeof(lpfnAcceptEx),
        &bytes, NULL, NULL);

    for (int i = 0; i < NUM_POST_ACCEPTS; ++i)
        PostAccept(g_listenSocket);

    // 1) 게임룸 업데이트용 스레드
    std::thread updateThread([]() {
        auto prev = std::chrono::steady_clock::now();
        while (g_running) {
            auto now = std::chrono::steady_clock::now();
            float dt = std::chrono::duration<float>(now - prev).count();
            prev = now;
            for (auto* room : activeRooms)
                room->Update(dt);
            std::this_thread::sleep_for(std::chrono::milliseconds(33));
        }
        });

    // 2) IOCP 워커 스레드
    std::vector<std::thread> workers;
    for (int i = 0; i < NUM_WORKER_THREADS; ++i)
        workers.emplace_back(WorkerThread, g_hIOCP);

    printf("Press Enter to exit.\n");
    getchar();

    // 3) 서버 종료 절차
    g_running = false;
    updateThread.join();

    for (int i = 0; i < NUM_WORKER_THREADS; ++i)
        PostQueuedCompletionStatus(g_hIOCP, 0, 0, nullptr);
    for (auto& th : workers)
        th.join();

    CloseHandle(g_hIOCP);
    closesocket(g_listenSocket);
    WSACleanup();
    return 0;
}
