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

#ifndef FORAY__NAV__COSTMAP_VIEW_HPP_
#define FORAY__NAV__COSTMAP_VIEW_HPP_

#include <cstdint>

#include "foray/nav/types.hpp"

namespace foray
{
namespace nav
{

/// 代价取值：0 = 自由，越大越难走。
/// @note 致命与未知都按"不可通行"处理，但分开编码，便于日志与测试区分。
enum CostValue : uint8_t
{
  kCostFree = 0,
  /// 偏好区：比自由更"便宜"，用来表达"这条路队里更想走"（例如隧道）。
  /// 只影响代价权重，不改变可通行性。
  kCostPreferred = 1,
  /// 未知空间（地图未标、或地图外）：保守视为不可通行。
  kCostUnknown = 253,
  /// 致死：禁行区、高地、动态障碍占位。
  kCostLethal = 255,
};

/// 代价图的**只读快照**。算法只读它，永不就地修改。
///
/// 由代价图构建器（静态层 + 禁行层 + 膨胀层 + 动态层）生成；
/// 规划与控制都拿同一份快照，避免"规划时用旧图、控制时用新图"。
class CostmapView
{
public:
  virtual ~CostmapView() = default;

  /// 分辨率（米/格）。2027 赛季默认 0.05。
  virtual double resolution() const = 0;

  /// 栅格宽度（格数）。
  virtual int width() const = 0;

  /// 栅格高度（格数）。
  virtual int height() const = 0;

  /// 栅格原点（场地坐标系，米）。指**第 (0,0) 格的左下角**。
  virtual Vec2 origin() const = 0;

  /// 取代价；越界返回 kCostUnknown。
  virtual uint8_t cost_at(int ix, int iy) const = 0;

  /// 世界坐标 → 栅格下标；越界返回 false。
  virtual bool world_to_grid(const Vec2 & world, int * ix, int * iy) const = 0;

  /// 栅格下标 → 世界坐标（该格中心）；越界返回 false。
  virtual bool grid_to_world(int ix, int iy, Vec2 * world) const = 0;

  /// 该格是否可通行（非致命、非未知）。
  bool passable(int ix, int iy) const
  {
    const uint8_t c = cost_at(ix, iy);
    return c != kCostLethal && c != kCostUnknown;
  }
};

}  // namespace nav
}  // namespace foray

#endif  // FORAY__NAV__COSTMAP_VIEW_HPP_
