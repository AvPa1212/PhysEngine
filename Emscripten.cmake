# --- Emscripten Build Configuration for PhysEngine Web ---

# Ensure we are actually using the Emscripten compiler
if(NOT EMSCRIPTEN)
    message(FATAL_ERROR "This file must be used with the Emscripten toolchain (emcmake).")
endif()

# 1. Output Settings
# Generates MomentumCore.js and MomentumCore.wasm
set(WEB_OUT_DIR "${CMAKE_SOURCE_DIR}/web_dist")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${WEB_OUT_DIR})

# 2. Emscripten Linker Flags
# --bind: Enables the C++/JS binding classes (Embind)
# -sWASM=1: Explicitly request WebAssembly output
# -sALLOW_MEMORY_GROWTH=1: Prevents crashes if simulation handles many tasks
# -sMODULARIZE=1: Wraps everything in a clean JS Promise-based module
# -sEXPORT_NAME: The name used in index.html to load the module
# -sEXPORTED_FUNCTIONS: C API bridge functions exported to JavaScript
# -sEXPORTED_RUNTIME_METHODS: Runtime helpers for calling C functions from JS
# -O3: Maximum optimization for production WebAssembly
set(EMSCRIPTEN_LINK_FLAGS
    "--bind"
    "-sWASM=1"
    "-sALLOW_MEMORY_GROWTH=1"
    "-sMODULARIZE=1"
    "-sEXPORT_NAME='PhysEngine'"
    "-sEXPORTED_FUNCTIONS=['_Task_Create','_Task_Destroy','_Task_SetPosition','_Task_SetVelocity','_Task_SetMass','_Task_GetPositionX','_Task_GetPositionY','_Task_GetVelocityX','_Task_GetVelocityY','_Task_GetMass','_Task_GetStepCount','_Engine_IntegrateClassical','_State_Serialize','_State_Deserialize']"
    "-sEXPORTED_RUNTIME_METHODS=['cwrap']"
    "-O3"
)

# 3. Apply flags to the target
# We assume the target name is 'MomentumCore' from your main CMakeLists.txt
target_link_options(MomentumCore PRIVATE ${EMSCRIPTEN_LINK_FLAGS})

# Change extension to .js for the web build
set_target_properties(MomentumCore PROPERTIES SUFFIX ".js")

message(STATUS "WebAssembly build configured. Output: ${WEB_OUT_DIR}/MomentumCore.js")
