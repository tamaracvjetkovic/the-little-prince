cmake_minimum_required(VERSION 3.17)
project(TheLittlePrince3D)

set(CMAKE_CXX_STANDARD 20)

# --- Platform Specific Setup ---
if (APPLE)
    include_directories("/opt/homebrew/include")
    link_directories("/opt/homebrew/lib")
elseif (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# --- Find Packages ---
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
# Assimp might not have a find_package on all systems, try pkg-config or just link if found in common paths
find_package(assimp QUIET)

# --- Include Directories ---
include_directories(src)

# --- Define Executable ---
add_executable(TheLittlePrince3D
        src/Main.cpp
        src/Util.cpp
)

# --- Link Libraries ---
target_link_libraries(TheLittlePrince3D
        OpenGL::GL
        GLEW::GLEW
        glfw
)

if (assimp_FOUND)
    target_link_libraries(TheLittlePrince3D assimp::assimp)
else()
    # Fallback for manual link if find_package fails but it's in brew paths
    target_link_libraries(TheLittlePrince3D assimp)
endif()

# macOS specific
if (APPLE)
    target_link_libraries(TheLittlePrince3D "-framework OpenGL")
endif()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shader DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
