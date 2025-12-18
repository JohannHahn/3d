cmake_minimum_required(VERSION 3.12)

project(3d)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(main main.cpp thirdparty/src/glad.c)

target_include_directories(main PRIVATE thirdparty/include)

if (MSVC)
    #target_compile_options(MyApp PRIVATE /std:c++latest)
    #target_link_libraries(plot raylib stdc++latest)
    target_compile_options(main PRIVATE /std:c++latest)
    target_link_libraries(main PRIVATE opengl32 winmm)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
    CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(main PRIVATE opengl32 winmm stdc++exp)
endif()
