# 🌀 Momentum: Quantum-Chaotic Physics Engine

**Momentum** is a high-performance, deterministic physics engine that bridges C++20 computational rigor with modern WebGL visualization.

It simulates a hybrid environment where **Classical Mechanics**, **Lorenz Chaos**, and **Quantum State Evolution** coexist — transforming a conventional productivity interface (a To-Do list) into a living thermodynamic system.

---

## 🚀 Core Philosophy

Traditional productivity software treats tasks as static strings.

**Momentum treats tasks as physical bodies.**

### 🔹 Mass

Larger tasks possess higher inertia and resist displacement.

### 🔹 Chaos

Unattended tasks enter a Lorenz attractor state and begin exhibiting nonlinear, chaotic motion.

### 🔹 Entropy

Systems naturally evolve toward disorder. Without applied “work” (user interaction), the quantum state collapses.

### 🔹 Thermodynamics

As active tasks increase, the system’s internal energy rises — visually represented as workspace “burnout.”

Momentum is not just a task manager.
It is a computational metaphor for energy, disorder, and control.

---

## 🛠 Tech Stack

| Layer           | Technology         | Purpose                                                          |
| --------------- | ------------------ | ---------------------------------------------------------------- |
| **Engine Core** | C++20              | Deterministic ODE solvers (RK4), matrix math, Lorenz integration |
| **Bridge**      | WebAssembly (WASM) | Near-native execution in browser via Emscripten                  |
| **Frontend**    | ReactJS            | Component-based UI state management                              |
| **Graphics**    | Three.js / WebGL   | Real-time 3D chaotic trajectory rendering                        |
| **Shaders**     | GLSL               | Custom “Quantum Glow” and “Collapse” post-processing             |

---

## 📁 Project Structure

```plaintext
PhysEngine/
├── core/                  # Platform-agnostic C++ Logic
│   ├── include/           # Headers (Task, Chaos, Quantum, Thermo)
│   └── src/               # Implementations (Numerical Integrators)
│
├── momentum-ui/           # ReactJS Application
│   ├── src/components/    # QuantumTask & Workspace Visuals
│   ├── src/hooks/         # WASM Lifecycle Management
│   └── public/web_dist/   # Compiled WASM Binaries (Engine Brain)
│
├── deploy_web.py          # Automation: Compiles C++ & Deploys to React
└── web_dist/              # Local build artifacts
```

---

## ⚡ Quick Start

### 1️⃣ Prerequisites

* Emscripten SDK (`emsdk`)
* Node.js & npm
* Python 3.10+

---

### 2️⃣ Build the Physics Engine

The Python script automates compilation from C++ to WebAssembly and moves output into the React asset directory.

```bash
python3 deploy_web.py
```

---

### 3️⃣ Initialize the Frontend

Install React dependencies and 3D rendering libraries:

```bash
cd momentum-ui
npm install
```

---

### 4️⃣ Launch the Workspace

Start the development server:

```bash
npm start
```

The application will be available at:

```
http://localhost:3000
```

---

## 🧪 Mathematical Models & Constants

### 🔹 Chaos Model

Lorenz System:

* σ = 10
* ρ = 28
* β = 8 / 3

---

### 🔹 Numerical Integration

4th Order Runge-Kutta (RK4)

```
dt = 1 / 60 seconds (fixed timestep)
```

Deterministic stepping ensures reproducibility across platforms.

---

### 🔹 Quantum Evolution

Unitary approximation via Taylor expansion:

```
U ≈ I − iHdt − 0.5H²dt²
```

Where:

* H = Hamiltonian matrix
* dt = fixed timestep

---

### 🔹 Entropy Model

Shannon Entropy:

```
S = − Σ pᵢ ln(pᵢ)
```

Entropy thresholds trigger stochastic quantum collapse events.

---

## 📝 Developer Rules

### Thread Safety

The WASM module is single-threaded.
All calls to `engine.update()` must occur within:

* The main React render loop, or
* A dedicated `requestAnimationFrame` loop.

---

### Memory Management

C++ objects created via `Task_Create()` must be tracked explicitly.

* Always validate pointers before calling getters/setters.
* Avoid orphaned allocations in the WASM heap.

---

### Determinism

Do **not** use `Math.random()` for physics-critical values.

All stochastic behavior must use the seeded RNG inside `SimulationEngine`.

---

## 🎯 Long-Term Goals

* Full cross-platform support (Web, Windows, Linux, Android)
* Deterministic simulation parity across all builds
* GPU-accelerated post-processing
* Expandable physics modules (fluid fields, field coupling, particle interactions)
* Research-grade numerical validation tools

---

## 👨‍💻 Author

**Avi Panchasara**
Virginia Tech

Momentum is a hybrid research implementation exploring the intersection of:

* Numerical physics
* Deterministic simulation
* Interactive user experience
* Computational thermodynamics metaphors
