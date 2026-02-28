cmake_minimum_required(VERSION 3.20)
project(PhysEngine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Output Management ---
# Force libraries and executables to the root of the /build folder 
# This makes it much easier for momentum_api.py to find the .so/.dll
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# --- Platform Specific Flags ---
if(MSVC)
    add_compile_options(/O2 /fp:precise /WX /W4 /D_CRT_SECURE_NO_WARNINGS)
else()
    # -fPIC is essential for shared libraries on Linux/WSL
    add_compile_options(-O3 -fPIC -Wall -Wextra -Werror)
endif()

# --- 1. THE CORE SHARED LIBRARY ---
# Collect all source files from /src
file(GLOB_RECURSE LIB_SOURCES "src/*.cpp")

# Create the MomentumCore shared library (MomentumCore.dll or libMomentumCore.so)
add_library(MomentumCore SHARED ${LIB_SOURCES})

# Ensure headers in /include are available to the library and anyone linking to it
target_include_directories(MomentumCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

# --- 2. EXECUTABLES ---
# We keep these separate from the library
add_executable(PhysEngine main.cpp)
add_executable(PhysicsTests tests/PhysicsTests.cpp)
add_executable(QuantumTests tests/QuantumTests.cpp)

# --- 3. LINKING ---
# Link the executables to our core library
target_link_libraries(PhysEngine PRIVATE MomentumCore)
target_link_libraries(PhysicsTests PRIVATE MomentumCore)
target_link_libraries(QuantumTests PRIVATE MomentumCore)

# --- 4. INSTALL (Optional but helpful) ---
# Useful if you want to deploy the library later

# At the bottom of CMakeLists.txt
if(EMSCRIPTEN)
    include(Emscripten.cmake)
endif()