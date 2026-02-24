cmake_minimum_required(VERSION 3.20)
project(PhysEngine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# MSVC specific flags for optimization, strict floating point, and warnings as errors
if(MSVC)
    add_compile_options(/O2 /fp:precise /WX /W4)
endif()

# Include directories
include_directories(include)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp" "main.cpp")

add_executable(PhysEngine ${SOURCES})

# Add Test Executables
add_executable(PhysicsTests tests/PhysicsTests.cpp)
add_executable(QuantumTests tests/QuantumTests.cpp)

# Link headers to tests
target_include_directories(PhysicsTests PRIVATE include)
target_include_directories(QuantumTests PRIVATE include)