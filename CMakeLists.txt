<<<<<<< Updated upstream
# ---------------------------------------------------------------------------------------------
# Linux build (tested target: Fedora 43, GCC + CMake). Lives on the Linux branch.
#
# The Windows build on the main branch links the prebuilt Visual Studio GLFW library in
# Dependencies/GLFW. On Linux we use the system GLFW and OpenGL packages instead, and compile everything else (GLAD, GLM, stb_image, Dear ImGui)
# from the bundled Dependencies folder, exactly as on Windows.
#
# Install the packages once:
#     sudo dnf install cmake gcc-c++ glfw-devel mesa-libGL-devel
#
# Configure, build and run from the project root:
#     cmake -S . -B build-linux -DCMAKE_BUILD_TYPE=Debug
#     cmake --build build-linux -j
#     cd build-linux && ./OpenGL
#
# The program loads its shaders and piece images with relative paths, so run it from the
# build folder (the copy_assets step below puts those assets next to the executable).
# ---------------------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.20)
project(OpenGL LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# This file lives in the project root.
set(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(DEPS      "${PROJECT_ROOT}/Dependencies")
# ---- System packages ------------------------------------------------------------------------
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)                  # mesa-libGL-devel (or your GPU vendor's driver)
find_package(glfw3 3.3 REQUIRED)               # glfw-devel, provides the 'glfw' target

# ---- Executable -----------------------------------------------------------------------------
add_executable(OpenGL
    "src/main.cpp"
    "Dependencies/glad/src/glad.c"
    "src/shader.h"
    "Dependencies/stb/stb_image.cpp"
    "src/ChessLogic.h" "src/testing.cpp" )

# Note: Dependencies/GLFW/include is deliberately NOT added here, so <GLFW/glfw3.h> comes
# from the system package and always matches the GLFW library we link against.
target_include_directories(OpenGL PRIVATE
    ${CMAKE_SOURCE_DIR}/Dependencies
    ${CMAKE_SOURCE_DIR}/Dependencies/GLFW/include
    ${CMAKE_SOURCE_DIR}/Dependencies/glm
    ${CMAKE_SOURCE_DIR}/Dependencies/stb
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(OpenGL PRIVATE
    glfw
    OpenGL::GL
    ${CMAKE_DL_LIBS}           # GLAD and the ImGui OpenGL loader use dlopen() to find libGL
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(OpenGL PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra>)
    # Third-party sources are not ours to clean up; keep their warnings out of the log.
    set_source_files_properties(
        "${DEPS}/glad/src/glad.c"
        "${DEPS}/stb/stb_image.cpp"
        PROPERTIES COMPILE_OPTIONS "-w")
endif()

# ---- Copy runtime assets next to the executable ---------------------------------------------
# A separate target that runs on every build, so edits to a shader or a piece image show up
# even when no C++ file changed (a POST_BUILD step only runs when the program is relinked).
add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_ROOT}/src/boardShaders" "$<TARGET_FILE_DIR:OpenGL>/boardShaders"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_ROOT}/src/pieceShaders" "$<TARGET_FILE_DIR:OpenGL>/pieceShaders"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_ROOT}/src/ChessPieces"  "$<TARGET_FILE_DIR:OpenGL>/ChessPieces"
    COMMENT "Copying shaders and piece images next to the executable"
)
add_dependencies(OpenGL copy_assets)

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

add_custom_command(TARGET OpenGL POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/Icons
        $<TARGET_FILE_DIR:OpenGL>/Icons
)

set_target_properties(OpenGL PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:OpenGL>
=======
cmake_minimum_required(VERSION 4.0)
project(OpenGL LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(OpenGL
    "src/main.cpp"
    "Dependencies/glad/src/glad.c"
    "src/shader.h"
    "Dependencies/stb/stb_image.cpp"
    "src/ChessLogic.h" "src/testing.cpp" )

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

add_custom_command(TARGET OpenGL POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/Icons
        $<TARGET_FILE_DIR:OpenGL>/Icons
)

set_target_properties(OpenGL PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:OpenGL>
>>>>>>> Stashed changes
)