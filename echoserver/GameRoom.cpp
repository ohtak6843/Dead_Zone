#include "GameRoom.h"
#include "server.h"
#include "protocol.h"
#include <sstream>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <cmath>
#include <limits>
#include <random>
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
constexpr float ST2_MIN_Y = -20;
constexpr float PLAYER_RADIUS = 30.0f;
constexpr float ZOMBIE_RADIUS = 30.0f;

// 감지 및 공격 반경
constexpr float DETECT_RADIUS = 500.0f;
constexpr float ATTACK_RADIUS = 100.0f;
const float   DETECT_RADIUS2 = DETECT_RADIUS * DETECT_RADIUS;
const float   ATTACK_RADIUS2 = ATTACK_RADIUS * ATTACK_RADIUS;

constexpr float BOSS_JUMP_COOLDOWN = 10.0f;  
constexpr float BOSS_JUMP_RADIUS = 700.0f; 
constexpr float BOSS_JUMP_OFFSET = 120.0f; 
constexpr float BOSS_JUMP_TIME = 0.8f; 
constexpr float G = 9.8f;

inline float GetGroundY(int currentStage) {
    return (currentStage == 2) ? ST2_MIN_Y : MAP_MIN_Y;
}

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
    //BroadcastSnapshots();
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
    if (bossPhaseRequested.exchange(false)) {
        StartBossPhase_Internal();
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
    const float groundY = GetGroundY(currentStage);

    for (auto& z : zombies) {
        if (z.isJumping) continue; // ← was isLeaping

        if (z.isAirborne) {
            z.y += z.verticalVelocity * dt;
            z.verticalVelocity -= G * dt;
            if (z.y <= groundY) { z.y = groundY; z.verticalVelocity = 0.0f; }
            else if (z.y < groundY) { z.y = groundY; }
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
    int  maxCount = (currentStage == 1) ? 10 : 20;
    int  batchSize = (currentStage == 1) ? 10 : 20;

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
            spawnY = ST2_MIN_Y;
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
        if (pct < 100)           type = ZombieType::BASIC;
        else if (pct < 0)      type = ZombieType::ELITE;
        else                    type = ZombieType::POLICE;

        // 3) 벡터에 추가
        Zombie z(type);
        z.wanderDirX = 0.0f;
        z.wanderDirZ = 0.0f;
        z.wanderTime = 0.0f;
        z.idleTime = 0.0f;
        z.x = spawnX;  z.y = spawnY;  z.z = spawnZ;
        z.id = nextZombieId++;
        zombies.push_back(z);

        // 4) 클라이언트로 패킷 전송
        sc_packet_spawn_zombie pkt{};
        pkt.size = sizeof(pkt);
        pkt.type = S2C_P_SPAWN_ZOMBIE;
        pkt.zombieId = z.id;
        pkt.position = { z.x, z.y + ((currentStage == 2) ? -10.0f : 0.0f), z.z };
        pkt.zombieType = static_cast<unsigned char>(type);

        for (auto* peer : players)
            PostSendPacket(peer, &pkt, pkt.size);
        // --------------------
    }
}

void GameRoom::UpdateZombies(float dt)
{
    const float ySendOffset = (currentStage == 2) ? -15.0f : 0.0f;
    HandleZombiePhysics(dt);
    HandleZombieCollisions();
    std::vector<sc_packet_zombie_snapshot::Entry> changed;
    changed.reserve(zombies.size());
    
    for (auto& z : zombies) {
        z.attackCooldown -= dt;
        if (z.attackCooldown < 0.0f) z.attackCooldown = 0.0f;

        if (z.type == ZombieType::BOSS) {
            if (z.isJumping) { 
                z.x += z.jumpVX * dt;
                z.z += z.jumpVZ * dt;
                z.y += z.jumpVY * dt;
                z.jumpVY -= G * dt;    
                z.jumpTime -= dt;

                const float groundY = GetGroundY(currentStage);
                bool landed = (z.y <= groundY) || (z.jumpTime <= 0.f);
                if (landed) {
                    z.y = groundY;
                    z.isJumping = false;
                    SetZombieState(z, Zombie::WALK); 

                    changed.push_back({ (uint32_t)z.id, Vector3{ z.x, z.y, z.z } });

                    const float waveR2 = 300.f * 300.f;
                    const int   waveDmg = 15;
                    for (auto* p : players) {
                        float dx = p->posX - z.x, dz = p->posZ - z.z;
                        if (dx * dx + dz * dz <= waveR2) {
                            int nh = p->health - waveDmg; if (nh < 0) nh = 0;
                            p->health = nh;

                            sc_packet_player_health hp{};
                            hp.size = sizeof(hp);
                            hp.type = S2C_P_PLAYER_HEALTH;
                            hp.playerId = p->socket;
                            hp.health = p->health;
                            for (auto* peer : players) PostSendPacket(peer, &hp, hp.size);
                        }
                    }
                }
                else {
                    changed.push_back({ (uint32_t)z.id, Vector3{ z.x, z.y, z.z } });
                }
                continue;
            }

            bossTimer -= dt;
            if (bossTimer <= 0.0f) {
                PER_SOCKET_CONTEXT* target = nullptr;
                float best2 = std::numeric_limits<float>::infinity();
                const float R2 = BOSS_JUMP_RADIUS * BOSS_JUMP_RADIUS;
                for (auto* p : players) {
                    float dx = p->posX - z.x;
                    float dz = p->posZ - z.z;
                    float d2 = dx * dx + dz * dz;
                    if (d2 <= R2 && d2 < best2) { best2 = d2; target = p; }
                }

                if (target) {
                    float ang = (rand() / (float)RAND_MAX) * 2.f * 3.14159265f;
                    float tx = target->posX + std::cos(ang) * BOSS_JUMP_OFFSET;
                    float tz = target->posZ + std::sin(ang) * BOSS_JUMP_OFFSET;
                    float ty = GetGroundY(currentStage);

                    const float T = BOSS_JUMP_TIME;
                    z.jumpVX = (tx - z.x) / T;
                    z.jumpVZ = (tz - z.z) / T;
                    z.jumpVY = (ty - z.y + 0.5f * G * T * T) / T;

                    z.isJumping = true;
                    z.jumpTime = T + 0.2f;           
                    SetZombieState(z, Zombie::ATTACK);  //임시   

                    bossTimer = BOSS_JUMP_COOLDOWN;
                    continue; 
                }
                else {
                    bossTimer = 0.2f; 
                }
            }
        }

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
            changed.push_back(
                sc_packet_zombie_snapshot::Entry{
        static_cast<uint32_t>(z.id),
        Vector3{ z.x, z.y + ySendOffset, z.z } 
                }
            );
            //BroadcastZombieMove(z, appliedDx, appliedDz);
        }
        else {
            if (z.wanderTime > 0.0f) {
                SetZombieState(z, Zombie::WALK);
                float dx = z.wanderDirX * z.walkSpeed * dt;
                float dz = z.wanderDirZ * z.walkSpeed * dt;
                z.x += dx;  z.z += dz;
                changed.push_back(
                    sc_packet_zombie_snapshot::Entry{
            static_cast<uint32_t>(z.id),
            Vector3{ z.x, z.y + ySendOffset, z.z }
                    }
                );
                //BroadcastZombieMove(z, dx, dz);
                z.wanderTime -= dt;
            }
            else if (z.idleTime > 0.0f) {
                SetZombieState(z, Zombie::IDLE);
                z.idleTime -= dt;
            }
            else {
                float angle = (rand() / (float)RAND_MAX) * 2.0f * 3.14159265f;
                z.wanderDirX = cosf(angle);
                z.wanderDirZ = sinf(angle);
                z.wanderTime = 5.0f + (rand() / (float)RAND_MAX) * 3.0f;
                z.idleTime = 1.0f + (rand() / (float)RAND_MAX) * 1.0f;
            }
        }
    }
    if (!changed.empty()) {
        constexpr size_t entrySz = sizeof(sc_packet_zombie_snapshot::Entry);          // 16
        constexpr size_t headerSz = offsetof(sc_packet_zombie_snapshot, entries);      // 3
        constexpr size_t maxPacket = 255;                                              // uint8_t 한계
        constexpr size_t maxEntries = (maxPacket - headerSz) / entrySz;                // =15

        // i는 changed 인덱스, 한 번에 maxEntries개씩 잘라 보낸다
        for (size_t i = 0; i < changed.size(); i += maxEntries) {
            size_t remain = changed.size() - i;
            // std::min 쓰지 않고 직접 비교
            size_t chunkCount = (remain > maxEntries) ? maxEntries : remain;

            size_t packetSz = headerSz + chunkCount * entrySz;
            char* buf = reinterpret_cast<char*>(malloc(packetSz));
            auto* pkt = reinterpret_cast<sc_packet_zombie_snapshot*>(buf);
            pkt->size = static_cast<uint8_t>(packetSz);
            pkt->type = S2C_P_ZOMBIE_SNAPSHOT;
            pkt->count = static_cast<uint8_t>(chunkCount);

            // 복사
            memcpy(pkt->entries, changed.data() + i, chunkCount * entrySz);

            for (auto* peer : players)
                PostSendPacket(peer, buf, packetSz);

            free(buf);
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

void GameRoom::SendAugmentOptions() {
    static const std::vector<uint8_t> allAugments = {
      0, // 플레이어 공격력 증강
      1, // 좀비 공격력 저하
      2, // 플레이어 최대체력 1.5배
      3, // 이동속도 +20%
      4, // 좀비 이동속도 감소 0.8배
    };

    std::vector<uint8_t> pool = allAugments;
    std::random_device rd;
    std::mt19937       gen(rd());
    std::shuffle(pool.begin(), pool.end(), gen);

    const size_t pickN = 3;
    augmentOptions.assign(pool.begin(), pool.begin() + pickN);

    hasSelected.clear();

    uint8_t cnt = static_cast<uint8_t>(augmentOptions.size());
    size_t  hdr = offsetof(sc_packet_augment_options, options);
    size_t  packetSize = hdr + cnt * sizeof(uint8_t);

    char* buf = reinterpret_cast<char*>(malloc(packetSize));
    auto* pkt = reinterpret_cast<sc_packet_augment_options*>(buf);
    pkt->size = static_cast<uint8_t>(packetSize);
    pkt->type = S2C_P_AUGMENT_OPTIONS;
    pkt->count = cnt;
    memcpy(pkt->options, augmentOptions.data(), cnt);

    for (auto* peer : players)
        PostSendPacket(peer, buf, packetSize);

    free(buf);
}

void GameRoom::HandleAugmentSelect(PER_SOCKET_CONTEXT* pContext, uint8_t idx) {
    if (hasSelected[pContext]) return;
    if (idx >= augmentOptions.size()) return;
    hasSelected[pContext] = true;

    uint8_t option = augmentOptions[idx];
    switch (option) {
    case 0: { // 플레이어 공격력 증강
        pContext->damage *= 1.5f;
        std::cout << "[서버] Player " << pContext->socket
            << " 공격력 x1.5 적용\n";
        break;
    }
    case 1: { // 좀비 공격력 저하
        for (auto& z : zombies) {
            z.attack = static_cast<int>(z.attack * 0.5f);
        }
        std::cout << "[서버] 좀비 공격력 x0.5 적용\n";
        break;
    }
    case 2: { // 플레이어 최대체력 1.5배 
        pContext->health = static_cast<int>(pContext->health * 1.5f);
        sc_packet_player_health hp{};
        hp.size = sizeof(hp);
        hp.type = S2C_P_PLAYER_HEALTH;
        hp.playerId = pContext->socket;
        hp.health = pContext->health;
        for (auto* peer : players) PostSendPacket(peer, &hp, hp.size);
        std::cout << "[서버] Player " << pContext->socket
            << " 최대체력 x1.5  (" << pContext->health << ")\n";
        break;
    }
    case 3: { // 이동속도 +20%
        pContext->walkSpeed *= 1.2f;
        std::cout << "[서버] Player " << pContext->socket
            << " 이동속도 +20% 적용\n";
        break;
    }
    case 4: { // 좀비 이동속도 감소
        for (auto& z : zombies) {
            z.walkSpeed = static_cast<int>(z.walkSpeed * 0.8f);
        }
        std::cout << "[서버] 좀비 이동속도 x0.8 적용\n";
        break;
    }
    default:
        std::cout << "[서버] 알 수 없는 증강체 옵션: " << int(option) << "\n";
        break;
    }
}

void GameRoom::QueueStartBossPhase(const Vector3& pos) {
    bossSpawnPos = pos;
    bossPhaseRequested = true;
}

void GameRoom::SpawnBoss(float x, float y, float z)
{
    Zombie boss(ZombieType::BOSS);
    boss.x = x; boss.y = y; boss.z = z;
    boss.id = nextZombieId++;
    boss.state = Zombie::IDLE;

    zombies.push_back(boss);
    bossId = boss.id;
    bossSpawned = true;
    bossTimer = 5.0f;

    sc_packet_spawn_zombie pkt{};
    pkt.size = sizeof(pkt);
    pkt.type = S2C_P_SPAWN_ZOMBIE;
    pkt.zombieId = boss.id;
    pkt.position = { boss.x, boss.y + ((currentStage == 2) ? -10.0f : 0.0f), boss.z };
    pkt.zombieType = static_cast<unsigned char>(ZombieType::BOSS);
    for (auto* peer : players) PostSendPacket(peer, &pkt, pkt.size);
}

void GameRoom::StartBossPhase_Internal()
{
    spawnPaused = true;

    for (const auto& z : zombies) {
        sc_packet_zombie_die diePkt{};
        diePkt.size = sizeof(diePkt);
        diePkt.type = S2C_P_ZOMBIE_DIE;
        diePkt.zombieId = z.id;
        for (auto* peer : players) PostSendPacket(peer, &diePkt, diePkt.size);
    }
    zombies.clear();

    SpawnBoss(bossSpawnPos.x, bossSpawnPos.y, bossSpawnPos.z);

    std::cout << "[서버] 보스 페이즈 시작! bossId=" << bossId << "\n";
}