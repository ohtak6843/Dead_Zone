#include "PhysicsSystem.h"

constexpr float STEP_VERTICAL_OFF = 10.0f;  // 발이 땅에 붙는 높이
constexpr float STEP_HEIGHT = 50.0f;  // 단차 허용 높이

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

    float cx = std::clamp(px, c.min[0], c.max[0]);
    float cz = std::clamp(pz, c.min[2], c.max[2]);
    float dx = px - cx, dz = pz - cz;
    float horizDist2 = dx * dx + dz * dz;

    // (2) 스텝업 로직: 수평 반지름 vs 단차, 발 높이는 STEP_VERTICAL_OFF 사용
    float sphereBottom = py - STEP_VERTICAL_OFF;
    float tileTop = c.max[1];
    float stepHeight = tileTop - sphereBottom;
    if (horizDist2 <= r * r
        && stepHeight > 0.0f
        && stepHeight <= STEP_HEIGHT)
    {
        // 발만 딱 STEP_VERTICAL_OFF 만큼 띄워서 계단 오르기
        py = tileTop + STEP_VERTICAL_OFF;
        return;
    }

    // 3) 기본 구-AABB 충돌 검사 (3D)
    //    (이때 y축 차이도 체크)
    float cy = std::clamp(py, c.min[1], c.max[1]);
    float dy = py - cy;
    float dist2 = dx * dx + dy * dy + dz * dz;
    if (dist2 > r * r)
        return;
    float dist = std::sqrt(dist2);
    if (dist <= 0.0f)
        return;

    // 4) 겹친 부분만큼 수평으로 밀어냄
    float overlap = r - dist;
    px += (dx / dist) * overlap;
    pz += (dz / dist) * overlap;
}
