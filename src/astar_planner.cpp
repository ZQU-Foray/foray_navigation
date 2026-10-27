// Copyright 2026 Foray
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "foray/nav/astar_planner.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

#include "foray/nav/costmap_view.hpp"

namespace foray
{
namespace nav
{
namespace
{

struct Node
{
  double f{0.0};
  int index{0};
};

struct NodeGreater
{
  bool operator()(const Node & a, const Node & b) const {return a.f > b.f;}
};

/// 膨胀层可能写到的最大代价（与 costmap.cpp 的 kMaxInflatedCost 对齐）。
constexpr uint8_t kMaxInflatedCostRef = 252;

/// 8 邻接的行走代价权重：膨胀格更"贵"，但不禁止（禁止靠致死格）。
double step_weight(uint8_t cost, double cost_weight)
{
  if (cost == kCostLethal) {
    return std::numeric_limits<double>::infinity();
  }
  if (cost == kCostUnknown) {
    return std::numeric_limits<double>::infinity();
  }
  const double normalized = static_cast<double>(cost) / static_cast<double>(kMaxInflatedCostRef);
  return 1.0 + cost_weight * std::min(1.0, normalized);
}

/// 八方向距离（octile）启发式，单位米。
double heuristic(int dx, int dy, double resolution)
{
  const int ax = std::abs(dx);
  const int ay = std::abs(dy);
  const int diagonal = std::min(ax, ay);
  const int straight = std::max(ax, ay) - diagonal;
  return (static_cast<double>(diagonal) * std::sqrt(2.0) + static_cast<double>(straight)) * resolution;
}

/// 保守的视线检测：沿线取足够密的采样点，任何一点不可通行就当挡。
bool line_of_sight(
  const CostmapView & map, const Vec2 & from, const Vec2 & to)
{
  const double dx = to.x - from.x;
  const double dy = to.y - from.y;
  const double length = std::sqrt(dx * dx + dy * dy);
  const double step = map.resolution() * 0.5;
  const int samples = std::max(2, static_cast<int>(std::ceil(length / std::max(1e-6, step))));
  for (int i = 0; i <= samples; ++i) {
    const double t = static_cast<double>(i) / static_cast<double>(samples);
    const Vec2 point{from.x + dx * t, from.y + dy * t};
    int ix = 0;
    int iy = 0;
    if (!map.world_to_grid(point, &ix, &iy)) {
      return false;
    }
    if (!map.passable(ix, iy)) {
      return false;
    }
  }
  return true;
}

}  // namespace

NavError AstarPlanner::plan(
  const CostmapView & map,
  const Pose2 & start,
  const Pose2 & goal,
  Path * out)
{
  if (out == nullptr) {
    return NavError::kInvalidInput;
  }
  out->clear();

  int start_x = 0;
  int start_y = 0;
  if (!map.world_to_grid(Vec2{start.x, start.y}, &start_x, &start_y)) {
    return NavError::kStartOutsideMap;
  }
  int goal_x = 0;
  int goal_y = 0;
  if (!map.world_to_grid(Vec2{goal.x, goal.y}, &goal_x, &goal_y)) {
    return NavError::kInvalidInput;   // 目标在地图外：调用方给的坐标有问题
  }
  if (!map.passable(start_x, start_y)) {
    return NavError::kInvalidInput;   // 起点在致死/未知格：先让上层把人放好
  }
  if (!map.passable(goal_x, goal_y)) {
    return NavError::kGoalInLethal;
  }

  const int width = map.width();
  const int height = map.height();
  if (width <= 0 || height <= 0) {
    return NavError::kInvalidInput;
  }
  const size_t total = static_cast<size_t>(width) * static_cast<size_t>(height);
  const double resolution = map.resolution();

  const int start_index = start_y * width + start_x;
  const int goal_index = goal_y * width + goal_x;
  if (start_index == goal_index) {
    out->push_back(Pose2{start.x, start.y, start.yaw});
    out->push_back(Pose2{goal.x, goal.y, goal.yaw});
    return NavError::kNone;
  }

  std::vector<double> g_score(total, std::numeric_limits<double>::infinity());
  std::vector<int> parent(total, -1);
  std::vector<char> closed(total, 0);
  std::priority_queue<Node, std::vector<Node>, NodeGreater> open;

  g_score[static_cast<size_t>(start_index)] = 0.0;
  open.push(Node{heuristic(goal_x - start_x, goal_y - start_y, resolution), start_index});

  const int kDx[8] = {1, -1, 0, 0, 1, 1, -1, -1};
  const int kDy[8] = {0, 0, 1, -1, 1, -1, 1, -1};

  int expansions = 0;
  bool found = false;
  while (!open.empty()) {
    const Node current = open.top();
    open.pop();
    if (closed[static_cast<size_t>(current.index)] != 0) {
      continue;
    }
    closed[static_cast<size_t>(current.index)] = 1;
    if (current.index == goal_index) {
      found = true;
      break;
    }
    if (++expansions > params_.max_expansions) {
      break;
    }

    const int cx = current.index % width;
    const int cy = current.index / width;
    for (int k = 0; k < 8; ++k) {
      const int nx = cx + kDx[k];
      const int ny = cy + kDy[k];
      if (nx < 0 || ny < 0 || nx >= width || ny >= height) {
        continue;
      }
      const int neighbor = ny * width + nx;
      if (closed[static_cast<size_t>(neighbor)] != 0) {
        continue;
      }
      const uint8_t cost = map.cost_at(nx, ny);
      if (cost == kCostLethal || cost == kCostUnknown) {
        continue;
      }
      const bool diagonal = (kDx[k] != 0) && (kDy[k] != 0);
      if (diagonal) {
        // 禁止斜穿犄角：两条正交邻格都得能走。
        const uint8_t side_a = map.cost_at(nx, cy);
        const uint8_t side_b = map.cost_at(cx, ny);
        if (side_a == kCostLethal || side_a == kCostUnknown ||
          side_b == kCostLethal || side_b == kCostUnknown)
        {
          continue;
        }
      }
      const double step = (diagonal ? std::sqrt(2.0) : 1.0) * resolution;
      const double candidate =
        g_score[static_cast<size_t>(current.index)] + step * step_weight(cost, params_.cost_weight);
      if (candidate < g_score[static_cast<size_t>(neighbor)]) {
        g_score[static_cast<size_t>(neighbor)] = candidate;
        parent[static_cast<size_t>(neighbor)] = current.index;
        open.push(
          Node{candidate + heuristic(goal_x - nx, goal_y - ny, resolution), neighbor});
      }
    }
  }

  if (!found) {
    return NavError::kNoPath;
  }

  // 回溯并转成世界坐标。
  std::vector<Vec2> cells;
  for (int index = goal_index; index != -1; index = parent[static_cast<size_t>(index)]) {
    const int ix = index % width;
    const int iy = index / width;
    Vec2 world{};
    if (!map.grid_to_world(ix, iy, &world)) {
      return NavError::kInvalidInput;
    }
    cells.push_back(world);
    if (index == start_index) {
      break;
    }
  }
  std::reverse(cells.begin(), cells.end());

  // 视线捷径化：能直连就跳过中间点。
  std::vector<Vec2> reduced;
  if (params_.smooth && cells.size() > 2) {
    reduced.push_back(cells.front());
    size_t anchor = 0;
    while (anchor + 1 < cells.size()) {
      size_t next = cells.size() - 1;
      while (next > anchor + 1 && !line_of_sight(map, cells[anchor], cells[next])) {
        --next;
      }
      reduced.push_back(cells[next]);
      anchor = next;
    }
    if (reduced.size() >= 2 && (reduced.back().x != cells.back().x ||
      reduced.back().y != cells.back().y))
    {
      reduced.push_back(cells.back());
    }
  } else {
    reduced = cells;
  }

  out->reserve(reduced.size() + 2);
  out->push_back(Pose2{start.x, start.y, start.yaw});
  for (const Vec2 & point : reduced) {
    out->push_back(Pose2{point.x, point.y, 0.0});
  }
  out->push_back(Pose2{goal.x, goal.y, goal.yaw});
  return NavError::kNone;
}

}  // namespace nav
}  // namespace foray
