# CMakeList.txt: WGCapturer 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#

cmake_minimum_required(VERSION 3.15)


project(WGCapturer)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows SDK 和 WinRT 支持
add_definitions(-DUNICODE -D_UNICODE)
add_compile_definitions(NOMINMAX)





include_directories()

add_executable(WGCapturer main.cpp WGCapturer.cpp "WGCapturer.h" "util.cpp" "util.h" "pch.cpp")

target_include_directories(WGCapturer PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/ext
    "D:\\softwares\\opencv\\opencv\\build\\include\\"
)


# 链接 DirectX 和 Windows 必需库
target_link_libraries(WGCapturer
    d3d11
    dxgi
    windowsapp
    "D:\\softwares\\opencv\\opencv\\build\\x64\\vc16\\lib\\opencv_world480.lib"
)



