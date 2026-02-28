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
# -lembind: Enables the C++/JS binding classes
# ALLOW_MEMORY_GROWTH: Prevents crashes if your simulation handles many tasks
# MODULARIZE: Wraps everything in a clean JS Promise-based module
set(EMSCRIPTEN_LINK_FLAGS 
    "--bind"
    "-s ALLOW_MEMORY_GROWTH=1"
    "-s MODULARIZE=1"
    "-s EXPORT_NAME='PhysEngine'"
    "-s EXPORTED_RUNTIME_METHODS=['ccall','cwrap']"
    "-O3"
)

# 3. Apply flags to the target
# We assume the target name is 'MomentumCore' from your main CMakeLists.txt
target_link_options(MomentumCore PRIVATE ${EMSCRIPTEN_LINK_FLAGS})

# Change extension to .js for the web build
set_target_properties(MomentumCore PROPERTIES SUFFIX ".js")

message(STATUS "WebAssembly build configured. Output: ${WEB_OUT_DIR}/MomentumCore.js")