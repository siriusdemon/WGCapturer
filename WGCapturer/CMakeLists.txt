cmake_minimum_required(VERSION 3.15)


project(WGCapturer)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DUNICODE -D_UNICODE)
add_compile_definitions(NOMINMAX)



add_library(WGCapturer SHARED WGCapturer.cpp "WGCapturer.h" "util.cpp" "util.h" "pch.cpp")

target_include_directories(WGCapturer PRIVATE 
    "D:\\softwares\\opencv\\opencv\\build\\include\\"
)


target_link_libraries(WGCapturer
    d3d11
    dxgi
    windowsapp
    "D:\\softwares\\opencv\\opencv\\build\\x64\\vc16\\lib\\opencv_world480.lib"
)



