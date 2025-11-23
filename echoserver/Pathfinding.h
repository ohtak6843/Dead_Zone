#pragma once
#include <vector>
#include <array>
#include <queue>
#include <unordered_map>
#include "Collider.h"

using Vec2f = std::array<float, 2>;
using GridPos = std::pair<int, int>;

// A* 노드 정보
struct PFNode {
    GridPos      pos;
    float        g, h;        // g: 시작→현재 비용, h: 휴리스틱 추정비용
    PFNode* parent = nullptr;
};

// 해시맵 키로 쓰기 위한 Hash, Equal
struct GridPosHash {
    size_t operator()(GridPos const& p) const noexcept {
        return std::hash<long long>()(((long long)p.first << 32) | (unsigned)p.second);
    }
};
struct GridPosEq {
    bool operator()(GridPos const& a, GridPos const& b) const noexcept {
        return a.first == b.first && a.second == b.second;
    }
};

class Pathfinding {
public:
    // 월드좌표 → 그리드 인덱스
    static GridPos WorldToGrid(const Vec2f& world, float cellSize, const Vec2f& origin) {
        int x = int((world[0] - origin[0]) / cellSize);
        int y = int((world[1] - origin[1]) / cellSize);
        return { x < 0 ? 0 : x, y < 0 ? 0 : y };
    }
    // 그리드 인덱스 → 월드좌표(셀 중앙)
    static Vec2f GridToWorld(const GridPos& gp, float cellSize, const Vec2f& origin) {
        return { origin[0] + (gp.first + 0.5f) * cellSize,
                 origin[1] + (gp.second + 0.5f) * cellSize };
    }

    // 장애물 콜라이더로부터 이동 가능/불가능 셀 맵 생성
    static std::vector<std::vector<bool>>
        BuildGrid(const std::vector<Collider>& cols,
            int widthCells, int heightCells,
            float cellSize, const Vec2f& origin)
    {
        std::vector<std::vector<bool>> grid(heightCells,
            std::vector<bool>(widthCells, true));
        for (const auto& c : cols) {
            // AABB 영역 [min,max]를 커버하는 그리드 인덱스 계산
            auto gmin = WorldToGrid({ c.min[0],c.min[2] }, cellSize, origin);
            auto gmax = WorldToGrid({ c.max[0],c.max[2] }, cellSize, origin);
            for (int gy = gmin.second; gy <= gmax.second && gy < heightCells; ++gy) {
                for (int gx = gmin.first; gx <= gmax.first && gx < widthCells; ++gx) {
                    if (gx >= 0 && gy >= 0)
                        grid[gy][gx] = false;
                }
            }
        }
        return grid;
    }

    // A* 경로 탐색: start,goal 월드좌표 → 리스트 반환
    static std::vector<Vec2f>
        FindPath(const Vec2f& start, const Vec2f& goal,
            const std::vector<std::vector<bool>>& grid,
            float cellSize, const Vec2f& origin)
    {
        int hCount = (int)grid.size(), wCount = (int)grid[0].size();
        auto toKey = [&](GridPos const& p) { return p; };
        auto heuristic = [&](GridPos a, GridPos b) {
            float dx = float(a.first - b.first);
            float dy = float(a.second - b.second);
            return std::sqrt(dx * dx + dy * dy);
            };

        GridPos startG = WorldToGrid(start, cellSize, origin);
        GridPos goalG = WorldToGrid(goal, cellSize, origin);

        // 우선순위 큐 (f = g+h 오름차순)
        struct QItem { PFNode* n; float f; };
        struct Cmp {
            bool operator()(QItem const& a, QItem const& b) const {
                return a.f > b.f;
            }
        };
        std::priority_queue<QItem, std::vector<QItem>, Cmp> openQ;
        std::unordered_map<GridPos, PFNode*, GridPosHash, GridPosEq> allNodes;
        std::unordered_map<GridPos, float, GridPosHash, GridPosEq> gScore;

        // 시작 노드
        PFNode* startN = new PFNode{ startG, 0.0f,
                         heuristic(startG,goalG), nullptr };
        openQ.push({ startN, startN->h });
        allNodes[startG] = startN;
        gScore[startG] = 0.0f;

        std::vector<GridPos> directions = {
            {1,0},{-1,0},{0,1},{0,-1}
        };

        PFNode* goalNode = nullptr;
        while (!openQ.empty()) {
            PFNode* cur = openQ.top().n; openQ.pop();
            if (cur->pos == goalG) { goalNode = cur; break; }

            for (auto& d : directions) {
                GridPos nb{ cur->pos.first + d.first,
                           cur->pos.second + d.second };
                if (nb.first < 0 || nb.second < 0 ||
                    nb.second >= hCount || nb.first >= wCount) continue;
                if (!grid[nb.second][nb.first]) continue; // 장애물

                float tentativeG = cur->g + 1.0f;
                if (!gScore.count(nb) || tentativeG < gScore[nb]) {
                    gScore[nb] = tentativeG;
                    float h = heuristic(nb, goalG);
                    PFNode* nbN = new PFNode{ nb, tentativeG, h, cur };
                    openQ.push({ nbN, tentativeG + h });
                    allNodes[nb] = nbN;
                }
            }
        }

        std::vector<Vec2f> path;
        if (goalNode) {
            for (PFNode* p = goalNode; p; p = p->parent) {
                path.push_back(GridToWorld(p->pos, cellSize, origin));
            }
            std::reverse(path.begin(), path.end());
        }

        // 메모리 정리
        for (auto& kv : allNodes) delete kv.second;
        return path;
    }
};
