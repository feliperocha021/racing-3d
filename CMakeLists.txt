cmake_minimum_required(VERSION 3.25)
project(Joguim C)

set(CMAKE_C_STANDARD 11)
include_directories(C:/MinGW/include)
link_directories(C:/MinGW/lib)

add_executable(Joguim main.c)

target_link_libraries(Joguim opengl32 glu32 freeglut winmm)
