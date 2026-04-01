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
    target_link_libraries(MomentumCore PUBLIC m)

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

    # Google Test for unit tests
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    # Prevent overriding the parent project's compiler/linker settings on Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    # RapidCheck for property-based tests
    FetchContent_Declare(
        rapidcheck
        URL https://github.com/emil-e/rapidcheck/archive/refs/heads/master.zip
    )
    FetchContent_MakeAvailable(rapidcheck)

    enable_testing()

    add_executable(Vector2Tests tests/Vector2Tests.cpp)
    target_include_directories(Vector2Tests PRIVATE ${INC_DIR})
    target_link_libraries(Vector2Tests PRIVATE GTest::gtest_main m)

    add_executable(TaskTests tests/TaskTests.cpp)
    target_include_directories(TaskTests PRIVATE ${INC_DIR})
    target_link_libraries(TaskTests PRIVATE GTest::gtest_main m)

    add_executable(ClassicalEngineTests tests/ClassicalEngineTests.cpp)
    target_include_directories(ClassicalEngineTests PRIVATE ${INC_DIR})
    target_link_libraries(ClassicalEngineTests PRIVATE GTest::gtest_main m)

    add_executable(PhysicsPropertiesTest tests/PhysicsPropertiesTest.cpp)
    target_include_directories(PhysicsPropertiesTest PRIVATE
        ${INC_DIR}
        ${rapidcheck_SOURCE_DIR}/extras/gtest/include
    )
    target_link_libraries(PhysicsPropertiesTest PRIVATE GTest::gtest_main rapidcheck m)

    add_executable(RK4IntegrationTest tests/RK4IntegrationTest.cpp)
    target_include_directories(RK4IntegrationTest PRIVATE ${INC_DIR})
    target_link_libraries(RK4IntegrationTest PRIVATE GTest::gtest_main m)

    add_executable(NumericalPropertiesTest tests/NumericalPropertiesTest.cpp)
    target_include_directories(NumericalPropertiesTest PRIVATE
        ${INC_DIR}
        ${rapidcheck_SOURCE_DIR}/extras/gtest/include
    )
    target_link_libraries(NumericalPropertiesTest PRIVATE GTest::gtest_main rapidcheck m)

    add_executable(SimulationEngineTests tests/SimulationEngineTests.cpp)
    target_include_directories(SimulationEngineTests PRIVATE ${INC_DIR})
    target_link_libraries(SimulationEngineTests PRIVATE GTest::gtest_main m)

    add_executable(SimulationPropertiesTest tests/SimulationPropertiesTest.cpp)
    target_include_directories(SimulationPropertiesTest PRIVATE
        ${INC_DIR}
        ${rapidcheck_SOURCE_DIR}/extras/gtest/include
    )
    target_link_libraries(SimulationPropertiesTest PRIVATE GTest::gtest_main rapidcheck m)

    add_executable(BridgeTests tests/BridgeTests.cpp)
    target_include_directories(BridgeTests PRIVATE ${INC_DIR})
    target_link_libraries(BridgeTests PRIVATE MomentumCore GTest::gtest_main)

    add_executable(RoundTripPropertiesTest tests/RoundTripPropertiesTest.cpp)
    target_include_directories(RoundTripPropertiesTest PRIVATE
        ${INC_DIR}
        ${rapidcheck_SOURCE_DIR}/extras/gtest/include
    )
    target_link_libraries(RoundTripPropertiesTest PRIVATE MomentumCore GTest::gtest_main rapidcheck)
    target_compile_options(RoundTripPropertiesTest PRIVATE -Wno-sign-compare)

    add_executable(PausePropertiesTest tests/PausePropertiesTest.cpp)
    target_include_directories(PausePropertiesTest PRIVATE
        ${INC_DIR}
        ${rapidcheck_SOURCE_DIR}/extras/gtest/include
    )
    target_link_libraries(PausePropertiesTest PRIVATE GTest::gtest_main rapidcheck m)

    add_executable(InvariantPropertiesTest tests/InvariantPropertiesTest.cpp)
    target_include_directories(InvariantPropertiesTest PRIVATE
        ${INC_DIR}
        ${rapidcheck_SOURCE_DIR}/extras/gtest/include
    )
    target_link_libraries(InvariantPropertiesTest PRIVATE GTest::gtest_main rapidcheck m)

    add_executable(StabilityPropertiesTest tests/StabilityPropertiesTest.cpp)
    target_include_directories(StabilityPropertiesTest PRIVATE
        ${INC_DIR}
        ${rapidcheck_SOURCE_DIR}/extras/gtest/include
    )
    target_link_libraries(StabilityPropertiesTest PRIVATE GTest::gtest_main rapidcheck m)

    add_executable(IntegrationTests tests/IntegrationTests.cpp)
    target_include_directories(IntegrationTests PRIVATE ${INC_DIR})
    target_link_libraries(IntegrationTests PRIVATE MomentumCore GTest::gtest_main)

    add_executable(BenchmarkTests tests/BenchmarkTests.cpp)
    target_include_directories(BenchmarkTests PRIVATE ${INC_DIR})
    target_link_libraries(BenchmarkTests PRIVATE MomentumCore GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(Vector2Tests)
    gtest_discover_tests(TaskTests)
    gtest_discover_tests(ClassicalEngineTests)
    gtest_discover_tests(PhysicsPropertiesTest)
    gtest_discover_tests(RK4IntegrationTest)
    gtest_discover_tests(NumericalPropertiesTest)
    gtest_discover_tests(SimulationEngineTests)
    gtest_discover_tests(SimulationPropertiesTest)
    gtest_discover_tests(BridgeTests)
    gtest_discover_tests(RoundTripPropertiesTest)
    gtest_discover_tests(PausePropertiesTest)
    gtest_discover_tests(InvariantPropertiesTest)
    gtest_discover_tests(StabilityPropertiesTest)
    gtest_discover_tests(IntegrationTests)
    gtest_discover_tests(BenchmarkTests)

endif()