# Contributing to PhysEngine

Thanks for contributing. This project mixes C++ physics code, WebAssembly bindings, and a React UI.

## Setup

1. Fork and clone the repository.
2. Install dependencies:
   - C++ toolchain + CMake
   - Python 3.10+
   - Node.js and npm
   - Emscripten SDK (for WASM builds)
3. Build and run locally:
   - Native tests from `build/` with CMake targets
   - UI in `momentum-ui/`

## Development Guidelines

- Keep physics behavior deterministic unless a feature explicitly requires controlled randomness.
- Add tests for bug fixes and non-trivial features.
- Keep bridge API changes synchronized across:
  - `include/physics/MomentumBridge.h`
  - `src/physics/MomentumBridge.cpp`
  - frontend hooks/workers under `momentum-ui/src/`
- Prefer small, focused pull requests.

## Pull Request Checklist

- [ ] Code builds locally
- [ ] Relevant tests pass (C++ and/or frontend)
- [ ] New behavior has tests or a clear justification
- [ ] Documentation updated when behavior or APIs change
- [ ] No unrelated refactors in the same PR

## Commit Messages

Use clear, imperative messages.

Examples:
- `Fix deadline force overflow in ClassicalEngine`
- `Add bridge getter for task internal energy`
- `Update worker to support pause and timescale`

## Reporting Issues

Use the issue templates and include:
- exact reproduction steps
- expected vs actual behavior
- platform details (OS, browser, compiler/toolchain)
- logs, screenshots, or failing test output when possible
