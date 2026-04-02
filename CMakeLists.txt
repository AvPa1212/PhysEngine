cmake_minimum_required(VERSION 3.20)
project(PhysEngine CXX)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Re-run configure when new source files are added under src/.
file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
set(INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(EMSCRIPTEN)
    message(STATUS "MOMENTUM: Initializing WebAssembly build path...")

    add_executable(MomentumCore ${LIB_SOURCES})
    target_include_directories(MomentumCore PUBLIC ${INC_DIR})
    target_compile_options(MomentumCore PRIVATE -O3 -Wall)

    set_target_properties(
        MomentumCore
        PROPERTIES
            LINK_FLAGS "--bind -sALLOW_MEMORY_GROWTH=1 -sMODULARIZE=1 -sEXPORT_NAME=\"PhysEngine\" -sEXPORTED_RUNTIME_METHODS=[\"ccall\",\"cwrap\"] -O3"
            SUFFIX ".js"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/web_dist"
    )
else()
    message(STATUS "MOMENTUM: Initializing desktop build path...")

    if(MSVC)
        set(MOMENTUM_COMMON_COMPILE_OPTIONS
            /O2
            /fp:precise
            /WX
            /W4
            /D_CRT_SECURE_NO_WARNINGS
        )
        set(MOMENTUM_MATH_LIBS)
    else()
        set(MOMENTUM_COMMON_COMPILE_OPTIONS
            -O3
            -fPIC
            -Wall
            -Wextra
            -Werror
        )
        set(MOMENTUM_MATH_LIBS m)
    endif()

    function(momentum_apply_native_defaults target)
        if(MOMENTUM_COMMON_COMPILE_OPTIONS)
            target_compile_options(${target} PRIVATE ${MOMENTUM_COMMON_COMPILE_OPTIONS})
        endif()
    endfunction()

    add_library(MomentumCore STATIC ${LIB_SOURCES})
    target_include_directories(MomentumCore PUBLIC ${INC_DIR})
    if(MOMENTUM_MATH_LIBS)
        target_link_libraries(MomentumCore PUBLIC ${MOMENTUM_MATH_LIBS})
    endif()
    momentum_apply_native_defaults(MomentumCore)

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

    add_executable(PhysEngine main.cpp)
    target_include_directories(PhysEngine PRIVATE ${INC_DIR})
    target_link_libraries(PhysEngine PRIVATE MomentumCore)
    momentum_apply_native_defaults(PhysEngine)

    add_executable(PhysicsTests tests/PhysicsTests.cpp)
    target_include_directories(PhysicsTests PRIVATE ${INC_DIR})
    target_link_libraries(PhysicsTests PRIVATE MomentumCore)
    momentum_apply_native_defaults(PhysicsTests)

    add_executable(QuantumTests tests/QuantumTests.cpp)
    target_include_directories(QuantumTests PRIVATE ${INC_DIR})
    target_link_libraries(QuantumTests PRIVATE MomentumCore)
    momentum_apply_native_defaults(QuantumTests)

    add_executable(SerializationTests tests/SerializationTests.cpp)
    target_include_directories(SerializationTests PRIVATE ${INC_DIR})
    target_link_libraries(SerializationTests PRIVATE MomentumCore)
    momentum_apply_native_defaults(SerializationTests)

    include(FetchContent)

    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    FetchContent_Declare(
        rapidcheck
        URL https://github.com/emil-e/rapidcheck/archive/refs/heads/master.zip
    )
    FetchContent_MakeAvailable(rapidcheck)

    enable_testing()
    include(GoogleTest)

    function(momentum_add_test target)
        set(options USE_CORE USE_RAPIDCHECK)
        set(one_value_args)
        set(multi_value_args SOURCES EXTRA_INCLUDES EXTRA_LIBS EXTRA_SOURCES EXTRA_COMPILE_OPTIONS)
        cmake_parse_arguments(MAT "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

        add_executable(${target} ${MAT_SOURCES} ${MAT_EXTRA_SOURCES})
        target_include_directories(${target} PRIVATE ${INC_DIR} ${MAT_EXTRA_INCLUDES})
        target_link_libraries(${target} PRIVATE GTest::gtest_main ${MAT_EXTRA_LIBS})

        if(MAT_USE_CORE)
            target_link_libraries(${target} PRIVATE MomentumCore)
        elseif(MOMENTUM_MATH_LIBS)
            target_link_libraries(${target} PRIVATE ${MOMENTUM_MATH_LIBS})
        endif()

        if(MAT_USE_RAPIDCHECK)
            target_include_directories(${target} PRIVATE ${rapidcheck_SOURCE_DIR}/extras/gtest/include)
            target_link_libraries(${target} PRIVATE rapidcheck)
        endif()

        momentum_apply_native_defaults(${target})

        if(MAT_EXTRA_COMPILE_OPTIONS)
            target_compile_options(${target} PRIVATE ${MAT_EXTRA_COMPILE_OPTIONS})
        endif()

        # Register tests at configure time instead of via generated discovery
        # wrappers. The current wrapper-based flow can degrade into
        # `*_NOT_BUILT` placeholder tests on this Windows toolchain even when
        # the executables and discovered test lists exist.
        gtest_add_tests(
            TARGET ${target}
            SOURCES ${MAT_SOURCES} ${MAT_EXTRA_SOURCES}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            TEST_LIST ${target}_TESTS
        )
    endfunction()

    momentum_add_test(ComplexTests SOURCES tests/ComplexTests.cpp)
    momentum_add_test(MatrixTests SOURCES tests/MatrixTests.cpp)
    momentum_add_test(ThermodynamicsEngineTests SOURCES tests/ThermodynamicsEngineTests.cpp)
    momentum_add_test(QuantumEngineTests SOURCES tests/QuantumEngineTests.cpp)
    momentum_add_test(ChaosEngineTests SOURCES tests/ChaosEngineTests.cpp)
    momentum_add_test(
        EulerIntegratorTests
        SOURCES tests/EulerIntegratorTests.cpp
        EXTRA_SOURCES src/physics/EnergyEngine.cpp
    )
    momentum_add_test(Vector2Tests SOURCES tests/Vector2Tests.cpp)
    momentum_add_test(TaskTests SOURCES tests/TaskTests.cpp)
    momentum_add_test(
        ClassicalEngineTests
        SOURCES tests/ClassicalEngineTests.cpp
        EXTRA_SOURCES src/physics/EnergyEngine.cpp
    )
    momentum_add_test(
        PhysicsPropertiesTest
        USE_RAPIDCHECK
        SOURCES tests/PhysicsPropertiesTest.cpp
        EXTRA_SOURCES src/physics/EnergyEngine.cpp
    )
    momentum_add_test(
        RK4IntegrationTest
        SOURCES tests/RK4IntegrationTest.cpp
        EXTRA_SOURCES src/physics/EnergyEngine.cpp
    )
    momentum_add_test(
        NumericalPropertiesTest
        USE_RAPIDCHECK
        SOURCES tests/NumericalPropertiesTest.cpp
        EXTRA_SOURCES src/physics/EnergyEngine.cpp
    )
    momentum_add_test(
        SimulationEngineTests
        SOURCES tests/SimulationEngineTests.cpp
        EXTRA_SOURCES src/physics/EnergyEngine.cpp
    )
    momentum_add_test(
        SimulationPropertiesTest
        USE_RAPIDCHECK
        SOURCES tests/SimulationPropertiesTest.cpp
        EXTRA_SOURCES src/physics/EnergyEngine.cpp
    )
    momentum_add_test(BridgeTests USE_CORE SOURCES tests/BridgeTests.cpp)
    momentum_add_test(
        RoundTripPropertiesTest
        USE_CORE
        USE_RAPIDCHECK
        SOURCES tests/RoundTripPropertiesTest.cpp
        EXTRA_COMPILE_OPTIONS $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wno-sign-compare>
    )
    momentum_add_test(
        PausePropertiesTest
        USE_RAPIDCHECK
        SOURCES tests/PausePropertiesTest.cpp
        EXTRA_SOURCES src/physics/EnergyEngine.cpp
    )
    momentum_add_test(
        InvariantPropertiesTest
        USE_RAPIDCHECK
        SOURCES tests/InvariantPropertiesTest.cpp
    )
    momentum_add_test(
        StabilityPropertiesTest
        USE_RAPIDCHECK
        SOURCES tests/StabilityPropertiesTest.cpp
        EXTRA_SOURCES src/physics/EnergyEngine.cpp
    )
    momentum_add_test(
        IntegrationTests
        USE_CORE
        SOURCES tests/IntegrationTests.cpp
    )
    momentum_add_test(
        BenchmarkTests
        USE_CORE
        SOURCES tests/BenchmarkTests.cpp
    )
    momentum_add_test(
        EnergyEngineTests
        SOURCES tests/EnergyEngineTests.cpp
        EXTRA_SOURCES src/physics/EnergyEngine.cpp
    )
    momentum_add_test(
        EnergyPropertiesTest
        USE_RAPIDCHECK
        SOURCES tests/EnergyPropertiesTest.cpp
        EXTRA_SOURCES src/physics/EnergyEngine.cpp
    )
    momentum_add_test(EulerVsRK4BenchmarkTest USE_CORE SOURCES tests/EulerVsRK4BenchmarkTest.cpp)
endif()
