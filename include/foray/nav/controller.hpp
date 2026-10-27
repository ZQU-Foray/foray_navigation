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

#ifndef FORAY__NAV__CONTROLLER_HPP_
#define FORAY__NAV__CONTROLLER_HPP_

#include "foray/nav/types.hpp"

namespace foray
{
namespace nav
{

/// 局部控制器接口：给当前位姿、一条路径和周期，出一个底盘速度指令。
///
/// 契约：
/// - **无状态规划、有状态控制**：控制器可以记上次的指令（做限幅、平滑），
///   `reset()` 之后必须回到初始状态；
/// - 输出前必须做**幅值限幅**与**变化率限幅**，不许把超限值交给底盘；
/// - 不抛异常；内部失败返回具体原因，由状态机决定是否进入恢复。
class LocalController
{
public:
  virtual ~LocalController() = default;

  /// 计算一个控制周期的指令。
  /// @param pose     当前位姿（世界坐标）
  /// @param path     待跟随的路径（含起点）
  /// @param dt       距上一周期的时长（秒）
  /// @param out      输出指令
  /// @return 成功为 `NavError::kNone`
  virtual NavError compute(
    const Pose2 & pose,
    const Path & path,
    double dt,
    VelocityCommand * out) = 0;

  /// 清空内部状态（切换目标、进入恢复后必须调用）。
  virtual void reset() = 0;
};

}  // namespace nav
}  // namespace foray

#endif  // FORAY__NAV__CONTROLLER_HPP_
