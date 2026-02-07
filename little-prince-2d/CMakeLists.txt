cmake_minimum_required(VERSION 3.17)
project(TheLittlePrince)

set(CMAKE_CXX_STANDARD 20)

# --- Platform Specific Setup ---
if (APPLE)
    # macOS (Homebrew) paths - ONLY used on Mac
    include_directories("/opt/homebrew/include")
    link_directories("/opt/homebrew/lib")
elseif (WIN32)
    # Windows specific settings
    # Defines usually needed to prevent warnings or link errors on Windows
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DGLEW_STATIC)
endif()

# --- Find Packages ---
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

# --- Define Executable ---
add_executable(TheLittlePrince
        src/Main.cpp
        src/Util.cpp
        src/Util.h
)

# --- Link Libraries ---
target_link_libraries(TheLittlePrince
        OpenGL::GL
        GLEW::GLEW
        glfw
)

# macOS specific: OpenGL is a Framework
if (APPLE)
    target_link_libraries(TheLittlePrince "-framework OpenGL")
endif()