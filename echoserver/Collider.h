#pragma once

#include <array>

struct Collider {
    std::array<float, 3> min;  // ÃÖ¼Ò ÁÂÇ¥ (x, y, z)
    std::array<float, 3> max;  // ÃÖ´ë ÁÂÇ¥ (x, y, z)
};