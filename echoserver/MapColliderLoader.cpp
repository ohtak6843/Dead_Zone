/* MapColliderLoader.cpp */
#include "MapColliderLoader.h"
#include <fstream>
#include <stdexcept>
#include "nlohmann/json.hpp"

std::vector<Collider> MapColliderLoader::Load(const std::string& jsonPath) {
    std::ifstream ifs(jsonPath);
    if (!ifs.is_open()) {
        throw std::runtime_error("MapColliderLoader: failed to open file '" + jsonPath + "'");
    }

    nlohmann::json j;
    try {
        ifs >> j;
    }
    catch (const nlohmann::json::parse_error& ex) {
        throw std::runtime_error(std::string("MapColliderLoader: JSON parse error: ") + ex.what());
    }

    if (!j.is_array()) {
        throw std::runtime_error("MapColliderLoader: JSON root element must be an array");
    }

    std::vector<Collider> cols;
    cols.reserve(j.size());
    for (const auto& elem : j) {
        if (!elem.contains("position") || !elem.contains("extents")) {
            throw std::runtime_error("MapColliderLoader: missing 'position' or 'extents' field");
        }
        const auto& posArr = elem["position"];
        const auto& extArr = elem["extents"];
        if (!posArr.is_array() || posArr.size() != 3 ||
            !extArr.is_array() || extArr.size() != 3) {
            throw std::runtime_error("MapColliderLoader: 'position' and 'extents' must be arrays of 3 floats");
        }

        Collider c;
        float height = 0.0f;
        for (int i = 0; i < 3; ++i) {
            float pos = posArr[i].get<float>();
            float ext = extArr[i].get<float>();
            c.min[i] = pos - ext;
            c.max[i] = pos + ext;
            if (i == 1) {
                // Y축(extents[1]) 기준 높이 계산
                height = ext * 2.0f;
            }
        }
        if (height <= 50.0f)
            continue;
        cols.push_back(c);
    }

    return cols;
}
