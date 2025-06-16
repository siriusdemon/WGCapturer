cmake_minimum_required(VERSION 3.15)


project(WGCapturer)



set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(tester main.cpp) 

target_link_libraries(tester WGCapturer)
target_include_directories(tester PRIVATE 
    "D:\\softwares\\opencv\\opencv\\build\\include\\"
)
