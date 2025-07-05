/* MapColliderLoader.h */
#pragma once

#include <string>
#include <vector>
#include "Collider.h"   // AABB 콜라이더 정의 포함

// MapColliderLoader: JSON 파일에서 콜라이더 데이터를 로드하는 클래스
class MapColliderLoader {
public:
    // jsonPath 경로에 있는 JSON 파일을 읽어 std::vector<Collider> 반환
    // JSON 형식: [{ "position": [x,y,z], "extents": [ex,ey,ez] }, ...]
    static std::vector<Collider> Load(const std::string& jsonPath);
};