```markdown
# 🌀 PhysEngine

**A deterministic hybrid classical–chaotic–quantum simulation engine written in C++20.**

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightgrey.svg)]()

PhysEngine is a modular numerical simulation framework that integrates:

- Classical time evolution
- Nonlinear chaotic dynamics (Lorenz system)
- Finite-dimensional quantum state evolution
- Real-time entropy computation

The engine is fully deterministic, numerically stable (RK4 integration), and designed for extensibility and scientific validation.

---

## 🚀 Core Capabilities

### Classical Mechanics
- Fixed timestep integration (`dt = 1/60 s`)
- Deterministic update loop
- Precision-controlled floating point model

### Chaotic Dynamics
- Lorenz attractor simulation
- 4th-order Runge–Kutta (RK4) integrator
- Sensitive dependence on initial conditions
- Stable fixed-step nonlinear evolution

### Quantum Evolution
- Finite-dimensional complex state vector (4D Hilbert space)
- Hermitian Hamiltonian structure
- Deterministic time propagation
- Probability extraction from amplitudes

### Information Theory
- Shannon entropy computed from:
S = - Σ p_i ln(p_i)
  
```

````

- Detects pure vs mixed quantum states

---

## 🧠 Mathematical Foundations

### 1. Lorenz Attractor

The chaotic subsystem follows:

\[
\frac{dx}{dt} = \sigma(y - x)
\]

\[
\frac{dy}{dt} = x(\rho - z) - y
\]

\[
\frac{dz}{dt} = xy - \beta z
\]

Constants:

- σ = 10  
- ρ = 28  
- β = 8/3  

Integrated using fixed-step RK4 for stability and deterministic reproducibility.

---

### 2. Quantum State Evolution

Time evolution follows the Schrödinger equation:

\[
i\hbar\frac{d\psi}{dt} = H\psi
\]

Where:

- ψ is a 4-dimensional complex state vector
- H is a Hermitian Hamiltonian matrix

Phase 1 supports diagonal Hamiltonians.  
Off-diagonal terms introduce state mixing and entropy growth.

---

## 🛠 Project Structure

```bash
PhysEngine/
├── CMakeLists.txt
├── include/
│   └── physics/
│       ├── Task.hpp
│       ├── SimulationEngine.hpp
│       └── MomentumBridge.h
│
├── src/physics/
│   └── MomentumBridge.cpp
│
├── main.cpp
├── build/          # Generated build files
└── README.md
````

---

## 📦 Build Instructions

### Requirements

* C++20 compliant compiler

  * MSVC 19.29+
  * GCC 10+
* CMake 3.20+

---

### Windows (Visual Studio)

```bash
git clone https://github.com/yourusername/PhysEngine.git
cd PhysEngine
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

Executable output:

```
build/Release/PhysEngine.exe
```

---

### Linux / WSL

```bash
mkdir build && cd build
cmake ..
make
./PhysEngine
```

---

## 🧪 Example Validation

Test configuration:

* Initial stress = (1.0, 1.0, 1.0)
* Diagonal Hamiltonian
* 100 frames at 60 Hz

Example output:

```
Frame 100 Validation:
Chaos X: -8.840121
Chaos Y: -7.384545
Entropy: 0.000000
```

Interpretation:

* Chaos values diverge and change sign (expected Lorenz behavior)
* Entropy remains zero (pure quantum state, no mixing)

---

## ⚙ Numerical Design Decisions

* Fixed timestep integration
* RK4 over Euler for nonlinear stability
* Deterministic update ordering
* No external runtime dependencies
* `/fp:precise` floating-point model (MSVC)

This ensures reproducible outputs across builds.

---

## 🗺 Roadmap

### Phase 1 (Complete)

* RK4 Lorenz integrator
* Quantum entropy calculation
* Deterministic engine core

### Phase 2 (In Progress / Optional)

* Off-diagonal Hamiltonian support
* Probability normalization enforcement
* Energy expectation value tracking

### Phase 3

* Multi-task coupling
* Entangled state evolution
* Adaptive timestep integration

---

## 🎯 Purpose

PhysEngine serves as:

* A numerical systems project
* A hybrid classical–quantum simulation experiment
* A foundation for future dynamical systems research

The architecture is intentionally modular to allow extension into:

* Coupled oscillators
* Thermodynamic modeling
* Quantum interaction networks
* GPU acceleration

---

## 👨‍💻 Author

**Avi Panchasara**
Electrical Engineering
Virginia Tech

---
