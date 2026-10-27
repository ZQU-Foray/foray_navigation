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

#ifndef FORAY__NAV__PLANNER_HPP_
#define FORAY__NAV__PLANNER_HPP_

#include "foray/nav/costmap_view.hpp"
#include "foray/nav/types.hpp"

namespace foray
{
namespace nav
{

/// 全局规划器接口：给起点、终点和一张只读代价图，出一条路径。
///
/// 契约：
/// - **纯函数式**：不持有状态、不记上次结果；同一输入必须给同一输出；
/// - 失败必须给出具体原因（`kNoPath` / `kStartOutsideMap` / `kGoalInLethal` / `kInvalidInput`）；
/// - 目标落在致死格时**不搜索**，直接 `kGoalInLethal`；
/// - 不修改 `CostmapView`；
/// - 不抛异常。
class GlobalPlanner
{
public:
  virtual ~GlobalPlanner() = default;

  /// 规划一条从 `start` 到 `goal` 的路径。
  /// @param map   只读代价快照
  /// @param start 起点（世界坐标）
  /// @param goal  目标（世界坐标）
  /// @param out   输出路径；成功时至少含起点与终点，失败时不保证内容
  /// @return 成功为 `NavError::kNone`
  virtual NavError plan(
    const CostmapView & map,
    const Pose2 & start,
    const Pose2 & goal,
    Path * out) = 0;
};

}  // namespace nav
}  // namespace foray

#endif  // FORAY__NAV__PLANNER_HPP_
