#include "PhysicsSystem.h"

bool PhysicsSystem::SphereAABBOverlap(
    const Collider& c,
    float px, float py, float pz,
    float r
) {
    // 각 축별로 구의 중심에서 AABB까지의 최단점 계산
    float cx = std::clamp(px, c.min[0], c.max[0]);
    float cy = std::clamp(py, c.min[1], c.max[1]);
    float cz = std::clamp(pz, c.min[2], c.max[2]);

    // 거리 제곱 계산
    float dx = px - cx;
    float dy = py - cy;
    float dz = pz - cz;
    return (dx * dx + dy * dy + dz * dz) <= (r * r);
}

void PhysicsSystem::ResolveCollision(
    float& px, float& py, float& pz,
    const Collider& c,
    float r
) {
    if (!SphereAABBOverlap(c, px, py, pz, r))
        return;

    // 충돌 벡터 및 거리 계산
    float cx = std::clamp(px, c.min[0], c.max[0]);
    float cy = std::clamp(py, c.min[1], c.max[1]);
    float cz = std::clamp(pz, c.min[2], c.max[2]);
    float dx = px - cx;
    float dy = py - cy;
    float dz = pz - cz;
    float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
    if (dist <= 0.0f)
        return;

    // 겹친 거리만큼 반발 벡터를 따라 위치 보정
    float overlap = r - dist;
    px += (dx / dist) * overlap;
    //py += (dy / dist) * overlap;
    pz += (dz / dist) * overlap;
}
