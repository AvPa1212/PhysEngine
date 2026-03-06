cmake_minimum_required(VERSION 3.20)
project(PhysEngine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- 1. SOURCE DISCOVERY ---
file(GLOB_RECURSE LIB_SOURCES "src/*.cpp")
set(INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

# --- 2. PLATFORM LOGIC (WEB vs DESKTOP) ---

if(EMSCRIPTEN)
    message(STATUS "MOMENTUM: Initializing WebAssembly Build Path...")

    # Emscripten requires an executable target to generate .js and .wasm
    add_executable(MomentumCore ${LIB_SOURCES})
    target_include_directories(MomentumCore PUBLIC ${INC_DIR})

    # We use a single string for LINK_FLAGS to ensure Emscripten parses every -s flag correctly.
    # --bind: Connects C++ and JS
    # -sMODULARIZE=1: Wraps the engine in a Promise-based function
    # -sEXPORT_NAME="PhysEngine": The name used in index.html
    set_target_properties(MomentumCore PROPERTIES 
        LINK_FLAGS "--bind -sALLOW_MEMORY_GROWTH=1 -sMODULARIZE=1 -sEXPORT_NAME=\"PhysEngine\" -sEXPORTED_RUNTIME_METHODS=[\"ccall\",\"cwrap\"] -O3"
        SUFFIX ".js"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/web_dist"
    )

    add_compile_options(-O3 -Wall)

else()
    message(STATUS "MOMENTUM: Initializing Desktop Shared Library Build Path...")

    # Shared Library for Python/WSL use
    add_library(MomentumCore SHARED ${LIB_SOURCES})
    target_include_directories(MomentumCore PUBLIC ${INC_DIR})

    # Standard Linux/Windows Compile Options
    if(MSVC)
        add_compile_options(/O2 /fp:precise /WX /W4 /D_CRT_SECURE_NO_WARNINGS)
    else()
        add_compile_options(-O3 -fPIC -Wall -Wextra -Werror)
    endif()

    # Desktop Output Management
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

    # Desktop Executables & Testing
    add_executable(PhysEngine main.cpp)
    add_executable(PhysicsTests tests/PhysicsTests.cpp)
    add_executable(QuantumTests tests/QuantumTests.cpp)
    add_executable(SerializationTests tests/SerializationTests.cpp)

    target_link_libraries(PhysEngine PRIVATE MomentumCore)
    target_link_libraries(PhysicsTests PRIVATE MomentumCore)
    target_link_libraries(QuantumTests PRIVATE MomentumCore)
    target_link_libraries(SerializationTests PRIVATE MomentumCore)

endif()