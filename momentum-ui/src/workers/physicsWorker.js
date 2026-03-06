/**
 * physicsWorker.js – Web Worker that runs the WASM physics simulation off the
 * main thread.
 *
 * Architecture:
 *   Main Thread  ──postMessage──▶  Worker (this file)
 *                ◀──postMessage──  Worker
 *
 * The simulation maintains a constant time-step dt ≈ 0.0166s (60 FPS) using
 * setInterval.  State updates and engine events are posted back to the main
 * thread each tick so the React layer can render without jank.
 *
 * Race-condition notes:
 *  - Task creation / destruction messages are processed synchronously inside
 *    the worker's single-threaded event loop, so the task map is always
 *    consistent when the simulation tick fires.
 *  - postMessage serialises its payload (structured clone), so there is no
 *    shared mutable state between threads.
 */

/* eslint-disable no-restricted-globals */

let Module = null;
const tasks = new Map(); // taskId → WASM Task pointer
let simInterval = null;

const STEP_MS = 1000 / 60; // ≈ 16.67 ms target
const ENTROPY_THRESHOLD = 1.5;

// ── Message handler ─────────────────────────────────────────────────────────

self.onmessage = async (e) => {
  const { type, ...data } = e.data;

  switch (type) {
    // ── Lifecycle ──────────────────────────────────────────────────────────

    case 'INIT': {
      try {
        // Import the Emscripten glue script (sets self.PhysEngine)
        importScripts(data.wasmPath);
        Module = await self.PhysEngine({
          locateFile: (path) =>
            path.endsWith('.wasm') ? data.wasmDir + path : path,
        });
        self.postMessage({ type: 'READY' });
        startSimulation();
      } catch (err) {
        self.postMessage({
          type: 'ERROR',
          message: `Worker WASM init failed: ${data.wasmPath} – ${err?.message || 'unknown error'}`,
        });
      }
      break;
    }

    case 'STOP':
      stopSimulation();
      break;

    // ── Task management ───────────────────────────────────────────────────

    case 'CREATE_TASK': {
      if (!Module) break;
      const ptr = Module.Task_Create();
      if (data.mass != null) Module.Task_SetMass(ptr, data.mass);
      if (data.stressX != null)
        Module.Task_SetStress(ptr, data.stressX, data.stressY, data.stressZ);
      tasks.set(data.taskId, ptr);
      self.postMessage({ type: 'TASK_CREATED', taskId: data.taskId });
      break;
    }

    case 'DESTROY_TASK': {
      const ptr = tasks.get(data.taskId);
      if (ptr) {
        Module.Task_Destroy(ptr);
        tasks.delete(data.taskId);
      }
      break;
    }

    // ── Physics commands ──────────────────────────────────────────────────

    case 'APPLY_FORCE': {
      const ptr = tasks.get(data.taskId);
      if (ptr && Module.Task_ApplyForce) {
        Module.Task_ApplyForce(ptr, data.fx, data.fy, data.fz);
      }
      break;
    }

    case 'SET_MASS': {
      const ptr = tasks.get(data.taskId);
      if (ptr) Module.Task_SetMass(ptr, data.mass);
      break;
    }

    case 'SET_STRESS': {
      const ptr = tasks.get(data.taskId);
      if (ptr) Module.Task_SetStress(ptr, data.sx, data.sy, data.sz);
      break;
    }

    case 'COLLAPSE': {
      const ptr = tasks.get(data.taskId);
      if (ptr) Module.Engine_PerformQuantumCollapse(ptr);
      break;
    }

    // ── Serialization ─────────────────────────────────────────────────────

    case 'SERIALIZE': {
      const ptr = tasks.get(data.taskId);
      if (ptr && Module.State_Serialize) {
        const json = Module.State_Serialize(ptr);
        self.postMessage({
          type: 'SERIALIZED',
          taskId: data.taskId,
          state: json,
        });
      }
      break;
    }

    case 'DESERIALIZE': {
      const ptr = tasks.get(data.taskId);
      if (ptr && Module.State_Deserialize) {
        Module.State_Deserialize(ptr, data.state);
      }
      break;
    }

    default:
      break;
  }
};

// ── Simulation loop ──────────────────────────────────────────────────────────

function startSimulation() {
  if (simInterval) return;

  simInterval = setInterval(() => {
    if (!Module || tasks.size === 0) return;

    const startTime = performance.now();
    const updates = {};
    const events = [];

    for (const [taskId, ptr] of tasks) {
      // Advance physics one deterministic step
      Module.Engine_UpdateChaos(ptr);

      // Read state for the UI
      const state = {
        stressX: Module.Task_GetStressX(ptr),
        stressY: Module.Task_GetStressY(ptr),
        stressZ: Module.Task_GetStressZ(ptr),
        entropy: Module.Task_GetEntropy(ptr),
        posX: Module.Task_GetPositionX(ptr),
        posY: Module.Task_GetPositionY(ptr),
        collapseProbability: Module.Task_GetCollapseProbability(ptr),
        stepCount: Module.Task_GetStepCount(ptr),
      };

      updates[taskId] = state;

      // ── Event detection ────────────────────────────────────────────────
      if (state.entropy > ENTROPY_THRESHOLD) {
        events.push({
          type: 'EntropyThresholdReached',
          taskId,
          entropy: state.entropy,
        });
        // Auto-collapse to reset entropy
        Module.Engine_PerformQuantumCollapse(ptr);
      }
    }

    const execTime = performance.now() - startTime;

    self.postMessage({
      type: 'STATE_UPDATE',
      tasks: updates,
      events,
      perf: { execTime, taskCount: tasks.size },
    });
  }, STEP_MS);
}

function stopSimulation() {
  if (simInterval) {
    clearInterval(simInterval);
    simInterval = null;
  }
  // Explicit cleanup – free every WASM allocation
  for (const [, ptr] of tasks) {
    if (Module) Module.Task_Destroy(ptr);
  }
  tasks.clear();
}
