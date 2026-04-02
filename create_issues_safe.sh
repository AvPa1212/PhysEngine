#!/usr/bin/env bash
set -e
REPO="AvPa1212/PhysEngine"
gh(){ "/mnt/c/Program Files/GitHub CLI/gh.exe" "$@"; }
create(){
  local title="$1"
  local body="$2"
  local label="${3:-enhancement}"
  if gh issue list --repo "$REPO" --state all --search "$title in:title" --limit 100 --json title --jq ".[].title" | grep -Fx -- "$title" >/dev/null 2>&1; then
    echo "SKIP - $title"
    return 0
  fi
  gh issue create --repo "$REPO" --title "$title" --body "$body" --label "$label"
  sleep 1
}create \
"Physics model toggles (CLASSICAL/ENERGY/THERMO/CHAOS/QUANTUM) are visual-only — simulation ignores them" \
"## Problem
In \`App.tsx\` the \`activeModels\` state tracks which physics models the user toggles on/off, but \`physicsWorker.ts\` always calls only \`Engine_UpdateChaos()\` per tick, regardless of these toggle states.

## Impact
Toggling CLASSICAL, QUANTUM, THERMO, ENERGY, or CHAOS buttons has **zero effect** on the actual physics. Users see different UI states but identical simulation behaviour.

## Fix Required
The worker must receive the active-model set in each tick (or on toggle change via a \`SET_MODELS\` message), then conditionally call:
- \`Engine_IntegrateClassical\` when CLASSICAL is on
- \`QuantumEngine::evolve\` when QUANTUM is on
- \`ThermodynamicsEngine::updateEntropy\` when THERMO is on
- \`Engine_UpdateChaos\` when CHAOS is on

## Files
- \`momentum-ui/src/App.tsx\` (activeModels state, lines ~88-94)
- \`momentum-ui/src/workers/physicsWorker.ts\` (simulation loop)
- \`momentum-ui/src/hooks/usePhysicsWorker.ts\` (worker API)" \
"bug"

create \
"\"Export State\" button silently writes to localStorage instead of downloading a file" \
"## Problem
In \`App.tsx\` the Export State button does:
\`\`\`ts
localStorage.setItem('momentum_export', JSON.stringify({ tasks, groups }))
\`\`\`
This silently writes to a hidden localStorage key with no user feedback and no file download — it is functionally invisible to users.

## Expected Behaviour
Clicking Export State should trigger a JSON file download via a Blob URL, e.g.:
\`\`\`ts
const blob = new Blob([JSON.stringify({ tasks, groups }, null, 2)], { type: 'application/json' });
const url  = URL.createObjectURL(blob);
const a    = document.createElement('a');
a.href     = url;
a.download = 'momentum-state.json';
a.click();
URL.revokeObjectURL(url);
\`\`\`

## Files
- \`momentum-ui/src/App.tsx\` line ~428" \
"bug"

create \
"useMomentum.js calls non-existent WASM bridge functions (Task_SetDeadline, Task_SetUrgency, etc.)" \
"## Problem
\`momentum-ui/src/hooks/useMomentum.js\` calls four functions that are **not exposed** in \`MomentumBridge.cpp\`:
- \`module.Task_SetDeadline()\`
- \`module.Task_SetUrgency()\`
- \`module.Task_SetKineticFriction()\`
- \`module.Task_SetStaticFriction()\`

The hook silently skips them via \`typeof module.Task_SetDeadline === 'function'\` guards, meaning **deadline and urgency are never actually set** on tasks even though the C++ engine uses them for force calculations.

## Impact
\`ClassicalEngine::integrateRK4\` computes deadline force as \`F = urgencyConstant / deadlineTime²\`, but both values stay at their C++ defaults (\`deadlineTime=10, urgencyConstant=100\`). User inputs are discarded.

## Fix Required
Add \`Task_SetDeadline\`, \`Task_SetUrgency\`, \`Task_SetKineticFriction\`, \`Task_SetStaticFriction\` to both the \`extern "C"\` block and \`EMSCRIPTEN_BINDINGS\` in \`MomentumBridge.cpp\`.

## Files
- \`momentum-ui/src/hooks/useMomentum.js\`
- \`src/physics/MomentumBridge.cpp\`" \
"bug"

create \
"QuantumEngine::evolve() is never called in the Web Worker — quantum state is frozen" \
"## Problem
\`physicsWorker.ts\`'s simulation loop calls only \`Module.Engine_UpdateChaos(ptr)\` per tick. \`Engine_IntegrateClassical\` and quantum evolution are never invoked.

The quantum wavefunction \`psi[4]\` is initialised in the Task constructor but **never updated** in the browser, making the entire \`QuantumEngine::evolve()\` implementation dead code at runtime.

## Impact
- \`collapseProbability\` (derived from \`psi\`) is always its initial value
- The quantum superposition model described in PLAN.md is never active
- \`Force Collapse\` button resets state that was never evolving

## Fix Required
Add a \`Engine_EvolveQuantum\` function to \`MomentumBridge.cpp\` and call it in the worker loop when the QUANTUM model toggle is active.

## Files
- \`momentum-ui/src/workers/physicsWorker.ts\`
- \`src/physics/MomentumBridge.cpp\`
- \`include/physics/QuantumEngine.hpp\`" \
"bug"

create \
"ThermodynamicsEngine::updateEntropy() is never called — real Shannon entropy is dead code" \
"## Problem
The worker simulation loop only calls \`Engine_UpdateChaos\`. \`ThermodynamicsEngine::updateEntropy()\`, which computes genuine Shannon entropy \`S = -S p·ln(p)\` from the quantum wavefunction, is never invoked.

The entropy values shown in the UI come entirely from \`ChaosEngine.hpp\` line 61:
\`\`\`cpp
task.entropy += (|deltaX| + |deltaY| + |deltaZ|) * 0.001;
\`\`\`
This is an unbounded running sum, not thermodynamic entropy.

## Impact
- The Analytics entropy metric is physically meaningless
- \`ThermodynamicsEngine\` is completely unused in production
- \`QuantumEngine::calculateCollapseProbability\` gives wrong results because it depends on the correct entropy value

## Fix Required
Expose \`Engine_UpdateThermodynamics\` via the bridge and call it in the worker loop when the THERMO model is active.

## Files
- \`momentum-ui/src/workers/physicsWorker.ts\`
- \`src/physics/MomentumBridge.cpp\`
- \`include/physics/ThermodynamicsEngine.hpp\`" \
"bug"

create \
"SimulationEngine::update() — the unified update method — is bypassed entirely and unused in production" \
"## Problem
\`include/core/SimulationEngine.hpp\` provides a \`update()\` method that correctly calls all four physics subsystems in sequence (Classical ? Quantum ? Thermodynamics ? Chaos). However, both the Web Worker and legacy frontend hooks call individual engine functions directly, bypassing this coordinated update.

The \`SimulationEngine\` class with its managed task \`std::vector\` is never instantiated in production.

## Impact
- Physics update order is inconsistent between subsystems
- Any future coupling logic placed in \`SimulationEngine::update()\` will silently not run
- The engine architecture described in PLAN.md (unified simulation loop) is not reflected in the running code

## Fix Required
Expose \`SimulationEngine\` methods through the bridge (or use per-task composite update), and ensure the WASM worker calls them.

## Files
- \`include/core/SimulationEngine.hpp\`
- \`momentum-ui/src/workers/physicsWorker.ts\`" \
"bug"

create \
"QuantumTask.tsx creates a full Three.js WebGLRenderer per task card — GPU context exhaustion" \
"## Problem
Each \`QuantumTask\` component instantiates its own:
- \`THREE.WebGLRenderer\`
- \`THREE.Scene\`
- \`THREE.PerspectiveCamera\`
- \`requestAnimationFrame\` loop

Browsers enforce a hard limit of 8–16 WebGL contexts per page. With more than ~8 tasks, earlier contexts are silently lost (the canvas goes black) and GPU memory leaks.

## Reproduction
Create 10+ tasks on the simulation page and observe black/blank task visualizations.

## Fix Required
Use a **single shared renderer** with multiple scenes or viewports, or replace per-card WebGL with CSS/SVG 3D animations. Three.js supports a \`ScissorTest\` pattern for rendering multiple viewports from a single renderer.

## Files
- \`momentum-ui/src/components/QuantumTask.tsx\`" \
"bug"

create \
"Duplicate hook files: useMomentum.js and useMomentum.ts coexist with incompatible signatures" \
"## Problem
Both \`momentum-ui/src/hooks/useMomentum.js\` and \`momentum-ui/src/hooks/useMomentum.ts\` exist in the same directory with completely different implementations:

| File | Returns |
|------|---------|
| \`useMomentum.js\` | \`{ tasks, createTask, deleteTask, updateSimulation, getTaskState }\` |
| \`useMomentum.ts\` | \`{ engine, isReady, error }\` (stub only) |

TypeScript module resolution may pick up either file, leading to silent type errors or wrong behaviour depending on the import path.

## Fix Required
Remove one of the files. The \`.js\` version has the full task management logic and should be retained (or converted to \`.ts\`). The stub \`.ts\` should be deleted.

## Files
- \`momentum-ui/src/hooks/useMomentum.js\`
- \`momentum-ui/src/hooks/useMomentum.ts\`" \
"bug"

create \
"README.md project structure diagram is wrong — does not match actual file layout" \
"## Problem
\`README.md\` describes the project as:
\`\`\`
PhysEngine/
+-- core/
¦   +-- include/
¦   +-- src/
\`\`\`
The actual layout is:
\`\`\`
PhysEngine/
+-- include/
+-- src/
\`\`\`
(No \`core/\` subdirectory exists.)

Additionally, \`momentum-ui/README.md\` is the **default Create React App boilerplate** and hasn't been updated. The project uses **Vite**, not CRA/webpack, so the instructions (\`npm start\`, CRA links) are misleading.

## Fix Required
1. Update the directory tree in \`README.md\` to reflect the actual layout
2. Replace \`momentum-ui/README.md\` with Vite-specific setup instructions

## Files
- \`README.md\`
- \`momentum-ui/README.md\`" \
"documentation"

create \
"System Heat and Total Energy use magic-number heuristics instead of physics values" \
"## Problem
Two key metrics in the UI are computed with arbitrary constants unrelated to physics:

**System Heat** (\`App.tsx\` line ~274):
\`\`\`ts
const systemHeat = tasks.length * 12.5;
\`\`\`

**Total Energy** (\`App.tsx\` line ~107):
\`\`\`ts
tasks.reduce((sum, task) => sum + task.difficulty * 8.6, 0)
\`\`\`

Neither \`12.5\` nor \`8.6\` corresponds to any physical quantity in the engine.

## Fix Required
- **System Heat**: Sum \`task.internalEnergy\` values from the worker's task states (exposed via the bridge)
- **Total Energy**: Compute as kinetic energy (\`½mv²\`) + potential energy (deadline force well) from actual simulation state

## Files
- \`momentum-ui/src/App.tsx\`
- \`src/physics/MomentumBridge.cpp\` (add \`Task_GetInternalEnergy\` if needed)" \
"bug"

create \
"Task deadlineTime counts down in simulation but is never displayed or warned about in the UI" \
"## Problem
\`ClassicalEngine::integrateRK4\` decrements \`task.deadlineTime -= dt\` every step (line 58 of \`ClassicalEngine.hpp\`). The deadline force grows as \`F = k / deadlineTime²\`, creating urgency — but this countdown is completely invisible in the UI.

There is no:
- Display of remaining deadline time per task
- Visual warning when a deadline approaches zero
- UI field for setting a task's deadline duration at creation time

## Impact
A core feature of the physics model (urgency-driven force from deadlines) is hidden from users. Tasks feel uniform regardless of their deadline.

## Fix Required
1. Expose \`Task_GetDeadlineTime\` via the bridge
2. Send deadline time in the worker's state updates
3. Display remaining time per task on the simulation and tasks pages
4. Add a deadline-setting input to the task creation form

## Files
- \`momentum-ui/src/components/QuantumTask.tsx\`
- \`momentum-ui/src/App.tsx\`
- \`src/physics/MomentumBridge.cpp\`" \
"enhancement"

# -- Missing features from PLAN.md --------------------------------------------

create \
"Phase-space visualization missing — simulation canvas only shows static dots with no trajectories" \
"## Problem
PLAN.md specifies Phase-space plots as a core visualization mode. The current simulation page renders tasks as plain SVG circles at their \`(posX, posY)\` position with no:
- Velocity vectors
- Trajectory trails / history paths
- Phase-space axes with meaningful labels
- Lorenz attractor path in \`(stressX, stressY, stressZ)\` space

## Required
Implement trajectory rendering by buffering the last N position/stress states per task and drawing them as fading polylines on the SVG canvas (or Three.js scene for the 3D Lorenz path).

## Files
- \`momentum-ui/src/App.tsx\` (simulation canvas section)
- \`momentum-ui/src/workers/physicsWorker.ts\` (include history in state updates)" \
"enhancement"

create \
"Lorenz attractor 3D trajectory visualization not implemented" \
"## Problem
README.md lists Three.js/WebGL for \"Real-time 3D chaotic trajectory rendering.\" There is no global Lorenz trajectory scene in the application. Only per-task mini spheres in separate WebGL contexts exist.

## Required
Implement a shared Three.js scene (replacing or supplementing the per-card renderers) that:
1. Draws the Lorenz attractor trajectory for each task as an evolving 3D \`THREE.Line\` using \`(stressX, stressY, stressZ)\` coordinates
2. Uses colour-coded trails to distinguish tasks
3. Responds to camera orbit (OrbitControls)

## Files
- \`momentum-ui/src/components/SimulationCanvas.jsx\` or a new dedicated 3D canvas component
- \`momentum-ui/src/App.tsx\`" \
"enhancement"

create \
"Analytics page is a stub — missing entropy trends, energy charts, and completion trajectories" \
"## Problem
PLAN.md specifies: productivity efficiency metrics, energy usage over time, entropy trends, and task completion trajectories. The current Analytics page shows only three static values:
- System Heat (heuristic, not physics-based)
- Average Difficulty
- Group Balance

No charts, no time series, no historical data.

## Required
- Line chart: entropy per task over time
- Line chart: total system energy over time  
- Bar chart: task difficulty distribution
- Task completion timeline with physics state at completion
- Use a lightweight chart library (recharts, chart.js) or SVG paths

## Files
- \`momentum-ui/src/App.tsx\` (analytics route)
- \`momentum-ui/src/workers/physicsWorker.ts\` (accumulate history)" \
"enhancement"

create \
"Pause / play simulation control is missing from the UI" \
"## Problem
\`SimulationEngine.hpp\` includes a \`paused\` boolean flag, but there is no pause button in the UI. The simulation runs continuously with no way to:
- Freeze time
- Inspect a snapshot of the current state
- Resume from a paused position

PLAN.md explicitly calls for \"pause/rewind simulation\" controls.

## Required
1. Add a \`PAUSE\` / \`RESUME\` message type to the worker protocol
2. When paused, halt the \`setInterval\` loop (or skip physics calls) but continue posting state
3. Add a Pause / Play button to the canvas controls in \`App.tsx\`

## Files
- \`momentum-ui/src/workers/physicsWorker.ts\`
- \`momentum-ui/src/hooks/usePhysicsWorker.ts\`
- \`momentum-ui/src/App.tsx\`" \
"enhancement"

create \
"Time-scaling (slow motion / fast forward) not implemented" \
"## Problem
PLAN.md specifies \"Adjustable time scaling: Slow motion (analysis) / Fast forward (prediction).\" Currently the worker uses a fixed \`STEP_MS = 1000/60\` (~16.67 ms) that cannot be changed at runtime.

## Required
1. Add a \`SET_TIMESCALE\` message to the worker protocol accepting a multiplier (e.g., 0.1×–10×)
2. Multiply \`Config::TIME_STEP\` by the scale factor before passing to engine calls (or adjust \`setInterval\` period)
3. Add a time-scale slider to the simulation page UI

## Files
- \`momentum-ui/src/workers/physicsWorker.ts\`
- \`momentum-ui/src/hooks/usePhysicsWorker.ts\`
- \`momentum-ui/src/App.tsx\`
- \`include/core/Config.hpp\`" \
"enhancement"

create \
"State persistence / simulation replay not implemented" \
"## Problem
PLAN.md specifies \"Save/load simulation states, Replay past productivity patterns, Compare runs.\"

Current Export button writes to localStorage (not a file). There is no:
- File download of simulation state
- Import/upload of a saved state
- Replay timeline
- State comparison view

## Required
1. Fix the Export button to trigger a real JSON file download
2. Add an Import button that reads a JSON file and restores tasks + physics state via the worker's \`DESERIALIZE\` message
3. Add a replay buffer that stores snapshots every N ticks
4. Add UI controls (scrubber) to step through replay

## Files
- \`momentum-ui/src/App.tsx\`
- \`momentum-ui/src/hooks/usePhysicsWorker.ts\`
- \`momentum-ui/src/workers/physicsWorker.ts\`" \
"enhancement"

create \
"Task dependency graph not implemented — no spring-force coupling between tasks" \
"## Problem
PLAN.md describes: \"Task Interaction Graph — Dependencies modeled as forces/springs, Coupled oscillations between tasks.\"

Currently all tasks evolve independently. There is no dependency relationship in the data model, no graph UI, and no inter-task force coupling in the C++ engine.

## Required
1. Add an optional \`dependsOn: string[]\` field to the Task UI type
2. In the worker, compute spring forces between dependent task pairs: \`F = -k × (distance - restLength)\`
3. Add a dependency-editing UI on the Tasks page
4. Render the dependency graph on the simulation canvas as connecting lines

## Files
- \`momentum-ui/src/App.tsx\`
- \`momentum-ui/src/workers/physicsWorker.ts\`
- \`include/physics/ClassicalEngine.hpp\` (optional C++ coupling)" \
"enhancement"

create \
"No CI/CD pipeline — missing GitHub Actions workflow for automated build and test" \
"## Problem
The repository has no \`.github/workflows/\` directory. There is no automated:
- C++ build (cmake/make)
- C++ unit test runner (PhysicsTests, QuantumTests, SerializationTests)
- React/TypeScript test runner (\`npm test\`)
- WASM compilation step

Every commit is deployed or merged without regression validation.

## Required
Create \`.github/workflows/ci.yml\` with jobs:
1. **cpp-tests**: Install GCC/CMake, build with \`cmake .. && make\`, run all GTest suites
2. **react-tests**: \`cd momentum-ui && npm ci && npm test -- --watchAll=false\`
3. (Optional) **wasm-build**: Install Emscripten, run \`bash build_web.sh\`

## Files
- \`.github/workflows/ci.yml\` (new)" \
"infrastructure"

create \
"WASM build artifacts not committed — Vercel deployment fails without MomentumCore.js / .wasm" \
"## Problem
\`README.md\` states: \"commit the generated files under \`momentum-ui/public/web_dist/\`.\"\`vercel.json\` expects them at build time but they are git-ignored (or absent). Vercel has no Emscripten toolchain, so it cannot build them during deployment.

This means **every Vercel deployment will fail** to serve the physics engine.

## Options
**A (Recommended):** Pre-commit the compiled WASM artifacts to the repo under \`momentum-ui/public/web_dist/\` and update \`.gitignore\` accordingly.

**B (CI-based):** Add a GitHub Actions workflow that compiles WASM and commits the output before Vercel builds.

## Files
- \`.gitignore\`
- \`vercel.json\`
- \`momentum-ui/public/web_dist/\`" \
"infrastructure"

create \
"No responsive / mobile layout — fixed 3-column grid breaks on screens < 800px" \
"## Problem
The app uses a fixed \`grid-template-columns: 220px 1fr 260px\` layout with no \`@media\` breakpoints. On mobile or narrow viewports the layout overflows horizontally, making the app unusable.

PLAN.md's UI philosophy emphasises \"minimalist but dynamic\" — which implies it should work across device sizes.

## Required
Add responsive breakpoints in \`index.css\`:
- At < 768px: collapse sidebars, show a hamburger menu or bottom tab bar
- At < 480px: single-column layout with scrollable panels

## Files
- \`momentum-ui/src/index.css\`" \
"enhancement"

create \
"GLSL \"Quantum Glow\" and \"Collapse\" post-processing shaders not implemented" \
"## Problem
README.md lists as part of the tech stack: \"GLSL / Custom 'Quantum Glow' and 'Collapse' post-processing.\" No GLSL shaders, post-processing passes, or visual collapse animations exist anywhere in the codebase.

## Required
Implement via Three.js \`EffectComposer\`:
1. **Quantum Glow**: Bloom/glow effect on task spheres that intensifies with entropy
2. **Collapse Effect**: Brief flash/implosion animation triggered when \`Force Collapse\` is called
3. Shader uniforms driven by real-time physics state (entropy, collapse probability)

## Files
- \`momentum-ui/src/components/QuantumTask.tsx\` (per-task visual collapse)
- New \`momentum-ui/src/shaders/\` directory" \
"enhancement"

create \
"No task completion history — completed tasks are permanently deleted with no analytics trail" \
"## Problem
When \"Complete Task\" is clicked in \`QuantumTask.tsx\` or \"Remove Task\" in \`App.tsx\`, the task is immediately deleted from state with no record of:
- When it was completed
- Its final entropy, energy, or stress values
- How many simulation steps it took
- Whether it was completed under high or low chaos

PLAN.md specifies \"Task completion trajectories\" in analytics.

## Required
1. Move completed tasks to a \`completedTasks\` array instead of deleting them
2. Store final physics state (snapshot from the worker at completion time)
3. Display completion history in the Analytics page with charts

## Files
- \`momentum-ui/src/App.tsx\`
- \`momentum-ui/src/hooks/usePhysicsWorker.ts\`" \
"enhancement"

create \
"Gamification layer not implemented — no physics-metric-based achievements" \
"## Problem
PLAN.md specifies: \"Achievements based on physical metrics: 'Low Entropy Week', 'High Energy Burst' / Visual rewards tied to system stability.\"

No achievement system, milestone tracking, badge display, or visual rewards exist in the codebase.

## Required
1. Define achievement definitions as rule objects: \`{ id, label, condition: (metrics) => boolean }\`
2. Evaluate conditions against real physics metrics (entropy, energy, task count, step count)
3. Persist earned achievements in localStorage
4. Display earned badges on the Analytics page with a notification on first unlock

## Files
- \`momentum-ui/src/App.tsx\`
- New \`momentum-ui/src/services/AchievementEngine.ts\`" \
"enhancement"

create \
"Goal Fields / long-term goal attractors not implemented" \
"## Problem
PLAN.md describes: \"Goal Fields — Long-term goals act as attractors, Tasks move toward goal basins.\"

No goal entity exists in the data model and no attractor force is applied to tasks. All tasks evolve under deadline forces and friction only; there is no teleological pull toward a user-defined goal.

## Required
1. Add a \`Goal\` type with a position in phase space and an attractor strength
2. In the C++ engine (or worker), compute \`F = -k × (taskPos - goalPos)\` for each task-goal pair
3. Add a Goals page to the UI for creating/managing goals
4. Render goal attractors as \"wells\" on the simulation canvas

## Files
- \`momentum-ui/src/App.tsx\`
- \`include/physics/ClassicalEngine.hpp\`
- \`src/physics/MomentumBridge.cpp\`" \
"enhancement"

create \
"Experimentation sandbox — physics model toggle buttons have no effect on the simulation" \
"## Problem
The simulation page's right sidebar shows CLASSICAL / ENERGY / THERMO / CHAOS / QUANTUM toggle pills. PLAN.md says the sandbox should allow comparing \"Pure classical vs Chaotic vs Quantum hybrid\" productivity outcomes.

Currently these toggles are stored in React state (\`activeModels\`) but are **never sent to the physics worker** and have no effect on which engines are active.

## Required
1. Send active model set to worker via a \`SET_ACTIVE_MODELS\` message
2. Worker conditionally applies each engine per tick
3. Add a visual indicator showing which models are currently contributing to the simulation
4. Consider adding a \"Compare\" mode that runs two parallel simulations side-by-side

## Files
- \`momentum-ui/src/App.tsx\`
- \`momentum-ui/src/hooks/usePhysicsWorker.ts\`
- \`momentum-ui/src/workers/physicsWorker.ts\`" \
"enhancement"

create \
"PerformanceMonitor data is collected but never displayed in the UI" \
"## Problem
\`PerformanceMonitor.ts\` tracks FPS, average frame time, C++ execution time per tick, and memory usage in a rolling 60-sample window. The \`usePhysicsWorker\` hook exposes it via \`perfMonitor\`.

However, no component in the application reads or renders any of these metrics. The data is collected silently and discarded.

## Required
Add a performance overlay or Analytics panel that displays:
- Live FPS from the physics worker
- Average C++ execution time per tick (ms)
- Memory usage (MB)
- Task count vs frame time graph

## Files
- \`momentum-ui/src/App.tsx\` (analytics page)
- \`momentum-ui/src/services/PerformanceMonitor.ts\`
- \`momentum-ui/src/hooks/usePhysicsWorker.ts\`" \
"enhancement"

create \
"Entropy accumulates unboundedly in ChaosEngine — value is a running sum, not Shannon entropy" \
"## Problem
In \`ChaosEngine.hpp\` line 61:
\`\`\`cpp
task.entropy += (std::abs(deltaX) + std::abs(deltaY) + std::abs(deltaZ)) * 0.001;
\`\`\`
This accumulates a running sum of Lorenz displacement magnitudes — it is **not Shannon entropy** and grows without bound (resetting only on quantum collapse).

Meanwhile \`ThermodynamicsEngine::updateEntropy()\` correctly computes \`S = -S p·ln(p)\` but is never called.

## Impact
- Entropy values shown to users are physically meaningless
- \`QuantumEngine::calculateCollapseProbability()\` divides by \`maxEntropy = ln(4) ˜ 1.386\`, but the running sum regularly exceeds this, returning clamped probability 1.0

## Fix Required
Replace the chaos-loop entropy accumulation with proper Shannon entropy computed from the quantum state after each \`ThermodynamicsEngine::updateEntropy()\` call.

## Files
- \`include/physics/ChaosEngine.hpp\`
- \`include/physics/ThermodynamicsEngine.hpp\`
- \`momentum-ui/src/workers/physicsWorker.ts\`" \
"bug"

create \
"Canvas controls (Collapse, Push +X, Pull) use hardcoded force vectors with no user control" \
"## Problem
The three canvas controls in \`App.tsx\` use hardcoded force values:
\`\`\`ts
applyForce(selectedTask.id, 1.5, 0.6, 0.2)   // Push +X
applyForce(selectedTask.id, -1.2, -0.3, 0)    // Pull
\`\`\`
Users have no control over force magnitude or direction.

PLAN.md specifies: \"Drag tasks (apply force), Inject energy (focus mode)\".

## Required
1. Replace hardcoded buttons with a force magnitude slider
2. Implement click-and-drag on the canvas SVG to apply a directional force in the drag vector
3. Add a \"Focus Mode\" that applies a sustained high-energy force to the selected task

## Files
- \`momentum-ui/src/App.tsx\` (canvas controls)
- \`momentum-ui/src/hooks/usePhysicsWorker.ts\`" \
"enhancement"

create \
"No keyboard accessibility — critical interactive elements lack keyboard navigation" \
"## Problem
The simulation page and task board have interactive elements (task selection, force controls, drag-and-drop board) that are not keyboard-accessible:

- Task list items (\`<button>\` in sidebar) rely on click only, no keyboard shortcuts
- Canvas force controls are mouse-only
- Drag-and-drop board has no keyboard alternative
- No focus outlines visible on interactive elements (CSS may be suppressing them)

## Required
1. Ensure all interactive elements are reachable via Tab
2. Add visible focus outlines (respecting \`prefers-reduced-motion\`)
3. Add keyboard shortcuts for common actions (e.g., N for new task, Space to pause)
4. Implement keyboard-based reordering as an alternative to drag-and-drop

## Files
- \`momentum-ui/src/App.tsx\`
- \`momentum-ui/src/index.css\`" \
"enhancement"

create \
"No error boundary — uncaught render errors crash the entire application" \
"## Problem
The React application has no error boundary component. If any physics state update causes an unexpected render error (e.g., NaN from the WASM engine, malformed state from the worker), the entire application unmounts with a blank white screen and no user-visible message.

## Required
Wrap the main \`<Workspace>\` component (and ideally each task card) in a React Error Boundary that:
1. Catches render/lifecycle errors
2. Displays a recovery UI (\"Simulation crashed — click here to reset\")
3. Logs the error to the console with physics state context

## Files
- \`momentum-ui/src/App.tsx\`
- New \`momentum-ui/src/components/ErrorBoundary.tsx\`" \
"enhancement"

create \
"Multi-Agent / collaborative mode not implemented" \
"## Problem
PLAN.md describes: \"Multi-Agent Mode — Multiple users = interacting systems, Collaboration modeled as force exchange.\"

The application is strictly single-user with no networking, real-time sync, or shared simulation state.

## Required (initial step)
1. Design the multi-agent data model (shared task pool, per-user force contributions)
2. Prototype using WebSockets or WebRTC for peer-to-peer state sync
3. Model collaboration as an attractive spring force between tasks owned by cooperating users
4. Model conflict as a repulsive force

This is a long-term feature. A good first step is documenting the architecture in a design doc.

## Files
- New architecture document under \`docs/\`
- \`momentum-ui/src/workers/physicsWorker.ts\` (force contribution API)" \
"enhancement"

create \
"Adaptive parameters (friction scaling, energy decay tuning) not implemented" \
"## Problem
PLAN.md specifies: \"Adaptive Parameters (Non-AI, Rule-Based) — Friction scaling with inactivity, Energy decay tuning based on workload, Threshold-triggered parameter shifts.\"

Currently all tasks have fixed \`staticFriction = 0.5\` and \`kineticFriction = 0.3\` set at creation time and never adjusted. There is no inactivity detection, workload monitoring, or threshold-triggered parameter mutation.

## Required
1. Track time-since-last-interaction per task in the worker
2. Increase \`kineticFriction\` (procrastination drag) when a task hasn't received user force for N ticks
3. Decrease \`urgencyConstant\` (focus decay) when system entropy is high
4. Expose parameter-adjustment messages via the worker API

## Files
- \`momentum-ui/src/workers/physicsWorker.ts\`
- \`include/physics/Task.hpp\`
- \`src/physics/MomentumBridge.cpp\`" \
"enhancement"

create \
"Calendar / real-world integration not implemented — deadlines have no external sync" \
"## Problem
PLAN.md describes: \"Real-World Integration — Calendar sync ? external forces, Deadlines ? potential wells, Notifications ? impulses.\"

Tasks have no connection to real calendar events. Deadlines are set manually as abstract difficulty values, not as actual dates and times.

## Required
1. Add a \`dueDate: Date | null\` field to tasks
2. Compute \`deadlineTime\` dynamically from \`(dueDate - now)\` in seconds
3. Integrate with browser Notification API to send impulse events on task interactions
4. Design (and optionally implement) a Google Calendar / iCal import flow

## Files
- \`momentum-ui/src/App.tsx\`
- \`momentum-ui/src/workers/physicsWorker.ts\`" \
"enhancement"

echo "All issues created!"
