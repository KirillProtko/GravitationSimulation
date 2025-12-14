cmake_minimum_required(VERSION 4.0)
project(GravitationSimulation)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(GravitationSimulation main.cpp
        Classes/object.cpp
        Classes/object.h
        Classes/camera.cpp
        Classes/camera.h
        Classes/grid2D.cpp
        Classes/grid2D.h
        Classes/star.cpp

)

add_library(glad STATIC ${CMAKE_SOURCE_DIR}/src/glad.c)

target_link_libraries(GravitationSimulation
        opengl32
        glfw3
        glad
)
if(WIN32)
    target_link_libraries(GravitationSimulation
            opengl32
            gdi32
    )
    add_definitions( -DGLFW_DLL)
endif()
