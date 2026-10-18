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

if(MSVC)
    target_link_options(OpenGLChessGame PRIVATE
        $<$<NOT:$<CONFIG:Debug>>:/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup>
    )

    # Non-Debug builds link the Microsoft C/C++ runtime statically (/MT), so the .exe does not
    # need VCRUNTIME140.dll / MSVCP140.dll and runs on PCs without the VC++ Redistributable.
    # Debug builds keep the normal DLL runtime (/MDd) for faster linking and debugging.
    set_property(TARGET OpenGLChessGame PROPERTY
        MSVC_RUNTIME_LIBRARY "$<IF:$<CONFIG:Debug>,MultiThreadedDebugDLL,MultiThreaded>"
    )
endif()

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

# GLFW has to be built with the same runtime as the program:
#   Debug     -> lib-vc2022/glfw3.lib     (built with the DLL runtime, /MD)
#   otherwise -> lib-vc2019/glfw3_mt.lib  (built with the static runtime, /MT; works with VS 2022)
set(GLFW_LIB    "${CMAKE_SOURCE_DIR}/Dependencies/GLFW/lib-vc2022/glfw3.lib")
set(GLFW_LIB_MT "${CMAKE_SOURCE_DIR}/Dependencies/GLFW/lib-vc2019/glfw3_mt.lib")

foreach(lib IN ITEMS "${GLFW_LIB}" "${GLFW_LIB_MT}")
    if(NOT EXISTS "${lib}")
        message(FATAL_ERROR "${lib} not found — check the Dependencies/GLFW folder")
    endif()
endforeach()

target_link_libraries(OpenGLChessGame PRIVATE
    $<IF:$<CONFIG:Debug>,${GLFW_LIB},${GLFW_LIB_MT}>
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