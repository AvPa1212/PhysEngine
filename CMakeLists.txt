cmake_minimum_required(VERSION 3.20)
project(PhysEngine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- 1. Platform Specific Flags ---
if(MSVC)
    add_compile_options(/O2 /fp:precise /WX /W4 /D_CRT_SECURE_NO_WARNINGS)
elseif(EMSCRIPTEN)
    # Web-specific optimization
    add_compile_options(-O3 -Wall) 
else()
    # Linux/WSL optimization
    add_compile_options(-O3 -fPIC -Wall -Wextra -Werror)
endif()

# --- 2. Source Discovery ---
file(GLOB_RECURSE LIB_SOURCES "src/*.cpp")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

# --- 3. Target Definition (Web vs Desktop) ---

if(EMSCRIPTEN)
    # --- WEB BUILD ---
    # Emscripten requires an 'executable' target to generate .js/.wasm output
    add_executable(MomentumCore ${LIB_SOURCES})
    
    # Apply the Emscripten-specific configurations from your Emscripten.cmake
    # This includes --bind, MODULARIZE, and memory settings
    include(Emscripten.cmake)
    
    message(STATUS "Configuring for WebAssembly build (MomentumCore.js)")

else()
    # --- DESKTOP BUILD (Linux/Windows) ---
    
    # Create the Shared Library for Python/Desktop use
    add_library(MomentumCore SHARED ${LIB_SOURCES})

    # Output Management (Keep binaries in the root of /build)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

    # Desktop Executables
    add_executable(PhysEngine main.cpp)
    add_executable(PhysicsTests tests/PhysicsTests.cpp)
    add_executable(QuantumTests tests/QuantumTests.cpp)

    # Linking
    target_link_libraries(PhysEngine PRIVATE MomentumCore)
    target_link_libraries(PhysicsTests PRIVATE MomentumCore)
    target_link_libraries(QuantumTests PRIVATE MomentumCore)
    
    message(STATUS "Configuring for Desktop Shared Library build")
endif()