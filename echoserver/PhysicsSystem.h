#pragma once
#include "Collider.h"
#include <algorithm>  
#include <cmath>      

// PhysicsSystem: 구(Sphere)와 AABB 충돌 처리 및 해제 기능
class PhysicsSystem {
public:
    // 충돌 감지: 구(center: px,py,pz 반지름 r)과 AABB 간의 충돌 여부 반환
    static bool SphereAABBOverlap(
        const Collider& c,
        float px, float py, float pz,
        float r
    );

    // 충돌 해제: 플레이어 위치(px,py,pz)를 충돌 바깥으로 이동

    static void ResolveCollision(
        float& px, float& py, float& pz,
        const Collider& c,
        float r
    );
};