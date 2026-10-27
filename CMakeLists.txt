cmake_minimum_required(VERSION 3.16)

project(foray_navigation VERSION 0.1.0 LANGUAGES CXX)

# 本仓只依赖标准库：不依赖 ROS 2、不依赖任何业务仓（见 .foray-layer）。
# 现阶段只有公共接口（header-only），实现随 M1–M6 逐步加入。

add_library(foray_navigation INTERFACE)
add_library(Foray::Navigation ALIAS foray_navigation)

target_include_directories(foray_navigation
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(foray_navigation INTERFACE cxx_std_17)

include(GNUInstallDirs)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
