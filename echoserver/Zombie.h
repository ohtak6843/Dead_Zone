#pragma once
#include <cmath>
#include <string>
#include <cstdio>
#include "Pathfinding.h"

// 좀비 종류 열거형
enum ZombieType {
    BASIC,              // 기본1 좀비
    POLICE,             // 경찰 좀비
    ELITE,              // 엘리트 좀비
    BOSS,               // 보스 좀비
    RUNNING,            // 뛰는 좀비
    CHARGER,            // 차저 좀비
    BOOMER,             // 부머 좀비
    HUNTER              // 헌터 좀비
};

// 보스 좀비의 상태 열거형 (보스 좀비에만 해당)
enum BossState {
    BOSS_NORMAL,      // 일반상태
    BOSS_PREAWAKEN,   // 각성준비상태 (15초간, 최대 HP의 20% 보호막 생성)
    BOSS_AWAKENED     // 각성상태 (체력 250000, 공격력 100, 공격속도 1.6회/s, 이동속도 4.5m/s, 받는 데미지 15% 감소)
};

class Zombie {
public:
    ZombieType type;
    long long  id;
    int health;
    int attack;
    float attackSpeed; // 초당 공격 횟수
    float walkSpeed;
    float runSpeed;
    float attackCooldown;
    bool  isAirborne = true;      // 땅을 벗어났는지
    float verticalVelocity = 0.0f;  // Y축 속도
    float wanderDirX = 0.0f;
    float wanderDirZ = 0.0f;
    float wanderTime = 0.0f;
    float idleTime = 0.0f;
    bool  isJumping = false;  
    float jumpVX = 0.f;     
    float jumpVY = 0.f;      
    float jumpVZ = 0.f;      
    float jumpTime = 0.f;     

    bool  isPreJump = false;
    float preJumpTimer = 0.f;
    float destX = 0.f, destY = 0.f, destZ = 0.f;

    bool  isScreaming = false;
    float screamTimer = 0.f;
    float screamWindup = 0.f;

    bool  isRecovering = false;
    float recoverTimer = 0.f;
    enum ZOMBIE_STATE : uint8_t {
        T_POSE = 0,
        IDLE = 1,
        WALK = 2,
        RUN = 3,
        ATTACK = 4,
        DIE = 5,
        JUMP=6,
        SCREAM=7,
        END = 8
    }state = IDLE;
    std::string specialSkill;

    // 현재 위치
    float x, y, z;
    std::vector<Vec2f> path;
    size_t             pathIdx = 0;
    float              speed = walkSpeed;
    // 생성자: 입력된 타입에 따라 스펙 초기화
    Zombie(ZombieType t) : type(t), x(0.0f), y(0.0f), z(0.0f) {
        if (t == BASIC) {
            health = 100;
            attack = 5;
            attackSpeed = 2.6f;
            walkSpeed = 30.0f;
            attackCooldown = 0.0f;
            specialSkill = "None";
		}
		else if (t == POLICE) {
			health = 200;
			attack = 5;
			attackSpeed = 2.6f;
			walkSpeed = 75.0f;
            attackCooldown = 0.0f;
			specialSkill = "None";
		}
        else if (t == ELITE) {
            health = 300;
            attack = 5;
            attackSpeed = 2.6f;
            walkSpeed = 75.0f;
            attackCooldown = 0.0f;
            specialSkill = "None";
        }
        else if (t == BOSS) {
            health = 50;
            attack = 8;
            attackSpeed = 2.6f;
            walkSpeed = 85.0f;
            attackCooldown = 0.0f;
            specialSkill = "Boss Basic Skill";
        }
    }

    std::pair<float, float> UpdatePosition(float dt, float targetX, float targetZ) {
        float dx = targetX - x;
        float dz = targetZ - z;
        float distance = std::sqrt(dx * dx + dz * dz);
        if (distance > 0.001f) {
            float vx = (dx / distance) * walkSpeed;
            float vz = (dz / distance) * walkSpeed;
            float moveX = vx * dt;
            float moveZ = vz * dt;
            x += moveX;
            z += moveZ;
            return { moveX, moveZ };
        }
        return { 0.f, 0.f };
    }
};
