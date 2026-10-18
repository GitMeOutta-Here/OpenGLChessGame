cmake_minimum_required(VERSION 4.0)
project(OpenGL LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(OpenGLChessGame
    "src/main.cpp"
    "Dependencies/glad/src/glad.c"
    "src/shader.h"
    "Dependencies/stb/stb_image.cpp"
    "src/ChessLogic.h" "src/testing.cpp" )

# Dependencies/glad/glad.h        -> #include <glad/glad.h>
# Dependencies/KHR/khrplatform.h  -> #include <KHR/khrplatform.h>
# Dependencies/GLFW/include/...   -> #include <GLFW/glfw3.h>
target_include_directories(OpenGLChessGame PRIVATE
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

target_link_libraries(OpenGLChessGame PRIVATE
    ${GLFW_LIB}
    opengl32
)

add_custom_command(TARGET OpenGLChessGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/boardShaders
        $<TARGET_FILE_DIR:OpenGLChessGame>/boardShaders
)

add_custom_command(TARGET OpenGLChessGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/pieceShaders
        $<TARGET_FILE_DIR:OpenGLChessGame>/pieceShaders
)

add_custom_command(TARGET OpenGLChessGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/ChessPieces
        $<TARGET_FILE_DIR:OpenGLChessGame>/ChessPieces
)

add_custom_command(TARGET OpenGLChessGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/Icons
        $<TARGET_FILE_DIR:OpenGLChessGame>/Icons
)

set_target_properties(OpenGLChessGame PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:OpenGLChessGame>
)