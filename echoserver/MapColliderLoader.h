#pragma once

#include <string>
#include <vector>
#include "Collider.h"   

class MapColliderLoader {
public:
    static std::vector<Collider> Load(const std::string& jsonPath);
};