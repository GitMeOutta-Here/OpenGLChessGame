cmake_minimum_required(VERSION 4.0)
project(OpenGL LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(OpenGL
    "src/main.cpp"
    "Dependencies/glad/src/glad.c"
    "x64/shader.h"
    "Dependencies/stb/stb_image.cpp"  
 "src/ChessLogic.h")

# Dependencies/glad/glad.h        -> #include <glad/glad.h>
# Dependencies/KHR/khrplatform.h  -> #include <KHR/khrplatform.h>
# Dependencies/GLFW/include/...   -> #include <GLFW/glfw3.h>
target_include_directories(OpenGL PRIVATE
    ${CMAKE_SOURCE_DIR}/Dependencies
    ${CMAKE_SOURCE_DIR}/Dependencies/GLFW/include
    ${CMAKE_SOURCE_DIR}/Dependencies/glm
    ${CMAKE_SOURCE_DIR}/Dependencies/stb
    ${CMAKE_SOURCE_DIR}/src
)

find_library(GLFW_LIB
    NAMES glfw3
    PATHS ${CMAKE_SOURCE_DIR}/Dependencies/GLFW/lib-vc2022
    NO_DEFAULT_PATH
)

if(NOT GLFW_LIB)
    message(FATAL_ERROR "glfw3.lib not found under Dependencies/GLFW/lib-vc2022 — check the path")
endif()

target_link_libraries(OpenGL PRIVATE
    ${GLFW_LIB}
    opengl32
)

add_custom_command(TARGET OpenGL POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/boardShaders
        $<TARGET_FILE_DIR:OpenGL>/boardShaders
)

add_custom_command(TARGET OpenGL POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/pieceShaders
        $<TARGET_FILE_DIR:OpenGL>/pieceShaders
)

add_custom_command(TARGET OpenGL POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/ChessPieces
        $<TARGET_FILE_DIR:OpenGL>/ChessPieces
)

set_target_properties(OpenGL PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:OpenGL>
)