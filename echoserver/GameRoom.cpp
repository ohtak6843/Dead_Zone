#include "GameRoom.h"
#include "server.h"
#include "protocol.h"
#include <sstream>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <cmath>
#include <limits>
#include "PhysicsSystem.h"

constexpr float MAP_MIN_X = 237.0f;
constexpr float MAP_MAX_X = 2030.0f;
constexpr float MAP_MIN_Y = 0.0f;
constexpr float MAP_MAX_Y = 960.0f;
constexpr float MAP_MIN_Z = -3552.0f;
constexpr float MAP_MAX_Z = 3535.0f;

constexpr float ST2_MIN_X = -2400.0f;
constexpr float ST2_MAX_X = 2800.0f;
constexpr float ST2_MIN_Z = 400.0f;
constexpr float ST2_MAX_Z = 3300.0f;

constexpr float PLAYER_RADIUS = 30.0f;
constexpr float ZOMBIE_RADIUS = 30.0f;

// 감지 및 공격 반경
constexpr float DETECT_RADIUS = 500.0f;
constexpr float ATTACK_RADIUS = 100.0f;
const float   DETECT_RADIUS2 = DETECT_RADIUS * DETECT_RADIUS;
const float   ATTACK_RADIUS2 = ATTACK_RADIUS * ATTACK_RADIUS;

std::vector<GameRoom*> activeRooms;

GameRoom::GameRoom(const std::vector<PER_SOCKET_CONTEXT*>& playersInput)
    : players(playersInput)
{
    activeRooms.push_back(this);
}

GameRoom::~GameRoom()
{
    auto it = std::find(activeRooms.begin(), activeRooms.end(), this);
    if (it != activeRooms.end())
        activeRooms.erase(it);
}

void GameRoom::Update(float dt)
{
    HandlePlayerPhysics(dt);
    HandlePlayerCollisions();
	SpawnZombies();
    UpdateZombies(dt);
    BroadcastSnapshots();
    // ─── 스테이지 전환 타이머 처리 ───
    if (stageChangeTimer >= 0.0f) {
        stageChangeTimer -= dt;
        if (stageChangeTimer <= 0.0f) {
            // 1) currentStage 갱신
            currentStage = nextStage;
            Vector3 startPos{ 2025.0f, 0.0f, 3974.0f };
            for (auto* pl : players) {
                pl->posX = startPos.x;
                pl->posY = startPos.y;
                pl->posZ = startPos.z;
            }
             mapColliders = MapColliderLoader::Load("../Resources/json/Stage02_Collider.json");
        std::cout << "Loaded colliders: " << mapColliders.size() << "\n";
         
            // 3) 클라이언트에 씬 전환 알림
            sc_packet_stage_change stagePkt{};
            stagePkt.size = sizeof(stagePkt);
            stagePkt.type = S2C_P_STAGE_CHANGE;
            stagePkt.newStage = (uint8_t)currentStage;
            for (auto* peer : players)
                PostSendPacket(peer, &stagePkt, stagePkt.size);
            // 5) 타이머 리셋
            stageChangeTimer = -1.0f;
        }
    }
    if (gameClearTimer >= 0.0f) {
        gameClearTimer -= dt;
        if (gameClearTimer <= 0.0f) {
            for (auto* victim : players) {
                sc_packet_player_leave leavePkt{};
                leavePkt.size = sizeof(leavePkt);
                leavePkt.type = S2C_P_PLAYER_LEAVE;
                leavePkt.playerId = victim->socket;  
                for (auto* p : players) {
                    PostSendPacket(p, &leavePkt, leavePkt.size);
                }
            }
            gameClearTimer = -1.0f;  // 재발 방지
        }
    }
}

void GameRoom::HandlePlayerPhysics(float dt)
{
    const float gravity = 9.8f;
    const float groundY = MAP_MIN_Y;

    for (auto* p : players) {
        p->posX += p->moveX * p->walkSpeed * dt;
        p->posZ += p->moveZ * p->walkSpeed * dt;

        if (p->isJumping) {
            p->posY += p->verticalVelocity * dt;
            p->verticalVelocity -= gravity * dt;

            if (p->posY <= groundY) {
                p->posY = groundY;
                p->verticalVelocity = 0.0f;
                //p->isJumping = false;
              //  SendLandPacket(p);
            }
            else if (p->posY < groundY) {
                p->posY = groundY;
            }
        }
        else if (p->posY < groundY) {
            p->posY = groundY;
        }
    }
}

void GameRoom::SendLandPacket(PER_SOCKET_CONTEXT* p)
{
    sc_packet_land landPkt;
    landPkt.size = sizeof(landPkt);
    landPkt.type = S2C_P_LAND;
    landPkt.playerId = p->socket;
    for (auto* peer : players) {
        if (peer != p)
            PostSendPacket(peer, &landPkt, landPkt.size);
    }
}

void GameRoom::HandlePlayerCollisions()
{
   // (1) 지면 바닥 Y만 남겨두고, 맵 경계는 mapColliders로 처리
    const float groundY = MAP_MIN_Y;
    for (auto* p : players) {
        // 지면 아래로 떨어지지 않도록 최소 높이 고정
        if (p->posY < groundY)
            p->posY = groundY;

        // 맵 콜라이더(AABB)과 구 충돌 해제
        // mapColliders는 server.cpp에서 MapColliderLoader로 로드된 전역 변수
        for (const auto& col : mapColliders) {
            PhysicsSystem::ResolveCollision(
                p->posX, p->posY, p->posZ,
                col,
                PLAYER_RADIUS
            );
        }
    }

    // (2) 플레이어끼리의 구-구 충돌 처리 
    size_t n = players.size();
    for (size_t i = 0; i < n; ++i) {
        auto* a = players[i];
        for (size_t j = i + 1; j < n; ++j) {
            auto* b = players[j];
            ResolveSphereCollision(
                a->posX, a->posY, a->posZ,
                b->posX, b->posY, b->posZ,
                PLAYER_RADIUS
            );
        }
    }
}

void GameRoom::HandleZombiePhysics(float dt)
{
    const float gravity = 9.8f;
    const float groundY = MAP_MIN_Y;

    for (auto& z : zombies) {
        if (z.isAirborne) {
            z.y += z.verticalVelocity * dt;
            z.verticalVelocity -= gravity * dt;

            if (z.y <= groundY) {
                z.y = groundY;
                z.verticalVelocity = 0.0f;
                //p->isJumping = false;
              //  SendLandPacket(p);
            }
            else if (z.y < groundY) {
                z.y = groundY;
            }
        }
        else if (z.y < groundY) {
            z.y = groundY;
        }
    }
}

// (2) 스텝업 & 벽 충돌 해제 + 착지 처리
void GameRoom::HandleZombieCollisions()
{
    // (1) 지면 바닥 Y만 남겨두고, 맵 경계는 mapColliders로 처리
    const float groundY = MAP_MIN_Y;
    for (auto& z : zombies) {
        // 지면 아래로 떨어지지 않도록 최소 높이 고정
        if (z.y < groundY)
            z.y = groundY;

        // 맵 콜라이더(AABB)과 구 충돌 해제
        // mapColliders는 server.cpp에서 MapColliderLoader로 로드된 전역 변수
        for (const auto& col : mapColliders) {
            PhysicsSystem::ResolveCollision(
                z.x, z.y, z.z,
                col,
                PLAYER_RADIUS
            );
        }
    }

    // (2) 플레이어끼리의 구-구 충돌 처리 
    size_t n = zombies.size();
    for (size_t i = 0; i < n; ++i) {
        auto& a = zombies[i];
        for (size_t j = i + 1; j < n; ++j) {
            auto& b = zombies[j];
            ResolveSphereCollision(
                a.x, a.y, a.z,
                b.x, b.y, b.z,
                PLAYER_RADIUS
            );
        }
    }
}
void GameRoom::ResolveSphereCollision(float& ax, float& ay, float& az,
    float& bx, float& by, float& bz,
    float radius)
{
    float dx = bx - ax;
    float dy = by - ay;
    float dz = bz - az;
    float dist2 = dx * dx + dy * dy + dz * dz;
    float minD = radius * 2.0f;
    if (dist2 < minD * minD && dist2 > 1e-6f) {
        float dist = std::sqrt(dist2);
        float pen = minD - dist;
        float nx = dx / dist;
        float ny = dy / dist;
        float nz = dz / dist;
        float half = pen * 0.5f;
        ax -= nx * half; ay -= ny * half; az -= nz * half;
        bx += nx * half; by += ny * half; bz += nz * half;
    }
}

void GameRoom::SpawnZombies()
{
    if (spawnPaused)
        return;

    auto now = std::chrono::steady_clock::now();

    // 스테이지별 최대 & 틱당 소환 개수
    int  maxCount = (currentStage == 1) ? 10 : 30;
    int  batchSize = (currentStage == 1) ? 1 : 10;

    // 이미 최대 마리수 도달했거나, 인터벌이 지나지 않았다면 리턴
    if ((int)zombies.size() >= maxCount || now - lastSpawn < spawnInterval)
        return;

    // 마지막 소환 시각 업데이트
    lastSpawn = now;

    // 이번 틱에 실제로 생성할 마리 수 계산
    int remaining = maxCount - static_cast<int>(zombies.size());
    int toSpawn = (batchSize < remaining) ? batchSize : remaining;
    for (int i = 0; i < toSpawn; ++i)
    {
        // --------------------
        // 1) 랜덤 위치 계산
        float spawnX, spawnZ, spawnY = 0.0f;
        if (currentStage == 1) {
            float xRange = (MAP_MAX_X - PLAYER_RADIUS) - (MAP_MIN_X + PLAYER_RADIUS);
            float zRange = (MAP_MAX_Z - PLAYER_RADIUS) - (MAP_MIN_Z + PLAYER_RADIUS);
            spawnX = MAP_MIN_X + PLAYER_RADIUS + (rand() / (float)RAND_MAX) * xRange;
            spawnZ = MAP_MIN_Z + PLAYER_RADIUS + (rand() / (float)RAND_MAX) * zRange;
        }
        else if (currentStage == 2) {
            float xRange = (ST2_MAX_X - PLAYER_RADIUS) - (ST2_MIN_X + PLAYER_RADIUS);
            float zRange = (ST2_MAX_Z - PLAYER_RADIUS) - (ST2_MIN_Z + PLAYER_RADIUS);
            spawnX = ST2_MIN_X + PLAYER_RADIUS + (rand() / (float)RAND_MAX) * xRange;
            spawnZ = ST2_MIN_Z + PLAYER_RADIUS + (rand() / (float)RAND_MAX) * zRange;
        }
        else {
            float xRange = (MAP_MAX_X - PLAYER_RADIUS) - (MAP_MIN_X + PLAYER_RADIUS);
            float zRange = (MAP_MAX_Z - PLAYER_RADIUS) - (MAP_MIN_Z + PLAYER_RADIUS);
            spawnX = MAP_MIN_X + PLAYER_RADIUS + (rand() / (float)RAND_MAX) * xRange;
            spawnZ = MAP_MIN_Z + PLAYER_RADIUS + (rand() / (float)RAND_MAX) * zRange;
        }

        // 2) 타입 확률
        int pct = rand() % 100;
        ZombieType type;
        if (pct < 70)           type = ZombieType::BASIC;
        else if (pct < 90)      type = ZombieType::ELITE;
        else                    type = ZombieType::POLICE;

        // 3) 벡터에 추가
        Zombie z(type);
        z.x = spawnX;  z.y = spawnY;  z.z = spawnZ;
        z.id = nextZombieId++;
        zombies.push_back(z);

        // 4) 클라이언트로 패킷 전송
        sc_packet_spawn_zombie pkt{};
        pkt.size = sizeof(pkt);
        pkt.type = S2C_P_SPAWN_ZOMBIE;
        pkt.zombieId = z.id;
        pkt.position = { z.x, z.y, z.z };
        pkt.zombieType = static_cast<unsigned char>(type);

        for (auto* peer : players)
            PostSendPacket(peer, &pkt, pkt.size);
        // --------------------
    }
}

void GameRoom::UpdateZombies(float dt)
{
    HandleZombiePhysics(dt);
    HandleZombieCollisions();
    for (auto& z : zombies) {
        z.attackCooldown -= dt;
        if (z.attackCooldown < 0.0f) z.attackCooldown = 0.0f;

        // 1) 가장 가까운 플레이어 찾기 
        PER_SOCKET_CONTEXT* nearest = nullptr;
        float bestDist2 = std::numeric_limits<float>::infinity();
        for (auto* p : players) {
            float dx = p->posX - z.x;
            float dz = p->posZ - z.z;
            float d2 = dx * dx + dz * dz;
            if (d2 < bestDist2) { bestDist2 = d2; nearest = p; }
        }

        // 2) 상태 전환 
        if (nearest && bestDist2 <= ATTACK_RADIUS2) {
            SetZombieState(z, Zombie::ATTACK);
          if (z.attackCooldown <= 0.0f) {
              z.attackCooldown = z.attackSpeed;
               const int dmg = z.attack;
               int newHp = nearest->health - dmg;
               nearest->health = (newHp > 0) ? newHp : 0;
                sc_packet_player_health hpPkt{};
                hpPkt.size = sizeof(hpPkt);
                hpPkt.type = S2C_P_PLAYER_HEALTH;
                hpPkt.playerId = nearest->socket;
                hpPkt.health = nearest->health;
                for (auto* peer : players)
                     PostSendPacket(peer, &hpPkt, hpPkt.size);
                /*if (nearest->health == 0) {
                    sc_packet_player_leave diePkt{};
                    diePkt.size = sizeof(diePkt);
                    diePkt.type = S2C_P_PLAYER_LEAVE;
                    diePkt.playerId = nearest->socket;
                    for (auto* peer : players)
                         PostSendPacket(peer, &diePkt, diePkt.size);
                    players.erase(
                            std::remove(players.begin(), players.end(), nearest),
                            players.end());    
                }*/
          }
        }
        else if (nearest && bestDist2 <= DETECT_RADIUS2) {
            SetZombieState(z, Zombie::WALK);

            // 3) 이동량 계산 (기존 z.UpdatePosition)
            auto [rawDx, rawDz] = z.UpdatePosition(dt, nearest->posX, nearest->posZ);

            // 4) 예측 위치에 물리 충돌 해제 적용
            float newX = z.x + rawDx;
            float newY = z.y;       
            float newZ = z.z + rawDz;
            /*for (const auto& col : mapColliders) {
                PhysicsSystem::ResolveCollision(
                    newX, newY, newZ,
                    col,
                    ZOMBIE_RADIUS
                );
            }*/

            // 5) 실제 적용된 이동량 재계산
            float appliedDx = newX - z.x;
            float appliedDz = newZ - z.z;

            // 6) 위치 갱신
            z.x = newX;
            z.z = newZ;
            // (z.y는 바닥 충돌 로직으로 이미 고정되어 있어야 합니다)

            // 7) 브로드캐스트
            BroadcastZombieMove(z, appliedDx, appliedDz);
        }
        else {
            SetZombieState(z, Zombie::IDLE);
        }
    }
}

void GameRoom::SetZombieState(Zombie& z, Zombie::ZOMBIE_STATE newState)
{
    if (z.state == newState) return;
    z.state = newState;
    sc_packet_zombie_state stPkt{};
    stPkt.size = sizeof(stPkt);
    stPkt.type = S2C_P_ZOMBIE_STATE;
    stPkt.zombieId = z.id;
    stPkt.state = static_cast<uint8_t>(z.state);
    for (auto* peer : players)
        PostSendPacket(peer, &stPkt, stPkt.size);
}

void GameRoom::BroadcastZombieMove(const Zombie& z, float dx, float dz)
{
    sc_packet_zombie_move mvPkt{};
    mvPkt.size = static_cast<unsigned char>(sizeof(mvPkt));
    mvPkt.type = S2C_P_ZOMBIE_MOVE;
    mvPkt.zombieId = z.id;
    mvPkt.position = { z.x, z.y, z.z };
    mvPkt.dx = dx;
    mvPkt.dz = dz;
    for (auto* peer : players)
        PostSendPacket(peer, &mvPkt, mvPkt.size);
}

//void GameRoom::ClampZombiePosition(Zombie& z)
//{
//    if (z.x - ZOMBIE_RADIUS < MAP_MIN_X)  z.x = MAP_MIN_X + ZOMBIE_RADIUS;
//    else if (z.x + ZOMBIE_RADIUS > MAP_MAX_X) z.x = MAP_MAX_X - ZOMBIE_RADIUS;
//    if (z.z - ZOMBIE_RADIUS < MAP_MIN_Z)  z.z = MAP_MIN_Z + ZOMBIE_RADIUS;
//    else if (z.z + ZOMBIE_RADIUS > MAP_MAX_Z) z.z = MAP_MAX_Z - ZOMBIE_RADIUS;
//    if (z.y < MAP_MIN_Y) z.y = MAP_MIN_Y;
//    else if (z.y > MAP_MAX_Y) z.y = MAP_MAX_Y;
//}

void GameRoom::BroadcastSnapshots()
{
    if (++snapshotFrameCount < snapshotFrameInterval) return;

    uint8_t count = static_cast<uint8_t>(players.size());
    size_t headerSize = offsetof(sc_packet_snapshot, entries);
    size_t entrySize = sizeof(sc_packet_snapshot::Entry);
    size_t totalSize = headerSize + count * entrySize;

    char* buf = reinterpret_cast<char*>(malloc(totalSize));
    if (!buf) {
        std::cerr << "[Error] snapshot buf alloc failed: " << totalSize << " bytes\n";
        snapshotFrameCount = 0;
        return;
    }

    auto* hdr = reinterpret_cast<sc_packet_snapshot*>(buf);
    hdr->size = static_cast<unsigned char>(totalSize);
    hdr->type = S2C_P_SNAPSHOT;
    hdr->count = count;

    for (int i = 0; i < count; ++i) {
        auto* p = players[i];
        hdr->entries[i].playerId = p->socket;
        hdr->entries[i].position = { p->posX, p->posY, p->posZ };
    }

    for (auto* peer : players)
        PostSendPacket(peer, buf, totalSize);
    free(buf);
    snapshotFrameCount = 0;
}

void GameRoom::RemoveZombieById(long long zombieId)
{
    auto it = std::find_if(zombies.begin(), zombies.end(),
        [zombieId](const Zombie& z) {
            return z.id == zombieId;
        });
    if (it != zombies.end()) {
        zombies.erase(it);
    }
}