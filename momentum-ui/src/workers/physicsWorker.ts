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

declare function importScripts(...urls: string[]): void;

const workerScope = self as any;
let Module: any = null;
const tasks = new Map<string, number>(); // taskId → WASM Task pointer
let simInterval: ReturnType<typeof setInterval> | null = null;

const STEP_MS = 1000 / 60; // ≈ 16.67 ms target
const ENTROPY_THRESHOLD = 1.5;
// Number of active tasks that triggers a SystemOverheat event from the worker.
const SYSTEM_OVERHEAT_TASK_COUNT = 8;

// ── Message handler ─────────────────────────────────────────────────────────

workerScope.onmessage = async (e: MessageEvent<any>) => {
  const { type, ...data } = e.data;

  switch (type) {
    // ── Lifecycle ──────────────────────────────────────────────────────────

    case 'INIT': {
      try {
        // Import the Emscripten glue script (sets self.PhysEngine)
        importScripts(data.wasmPath);
        const resolveFactory = async () => {
          const candidates = [
            workerScope.PhysEngine,
            workerScope.Module,
            workerScope.createModule,
            workerScope.MomentumCore,
          ];
          const globalFactory =
            candidates.find((factory: unknown) => typeof factory === 'function') ||
            candidates
              .map((candidate: any) => candidate?.default)
              .find((factory: unknown) => typeof factory === 'function') ||
            null;

          if (globalFactory) {
            return globalFactory;
          }

          const response = await fetch(data.wasmPath, { cache: 'no-store' });
          const source = await response.text();
          const evaluatedFactory = new Function(
            `${source}\nreturn (typeof PhysEngine === "function" && PhysEngine) || (typeof Module === "function" && Module) || (typeof createModule === "function" && createModule) || (typeof MomentumCore === "function" && MomentumCore) || null;`
          )();

          if (typeof evaluatedFactory === 'function') {
            return evaluatedFactory;
          }

          throw new Error(
            `No callable Emscripten factory found. HTTP ${response.status} content-type=${response.headers.get('content-type') || 'unknown'}`
          );
        };

        const engineFactory = await resolveFactory();

        Module = await engineFactory({
          locateFile: (path: string) =>
            path.endsWith('.wasm') ? data.wasmDir + path : path,
        });
        workerScope.postMessage({ type: 'READY' });
        startSimulation();
      } catch (err) {
        workerScope.postMessage({
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
      if (data.stressX != null || data.stressY != null || data.stressZ != null) {
        const sx = data.stressX ?? 0;
        const sy = data.stressY ?? 0;
        const sz = data.stressZ ?? 0;
        Module.Task_SetStress(ptr, sx, sy, sz);
      }
      tasks.set(data.taskId, ptr);
      workerScope.postMessage({ type: 'TASK_CREATED', taskId: data.taskId });
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
        workerScope.postMessage({
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
    const updates: Record<string, any> = {};
    const events: Array<{ type: string; [key: string]: any }> = [];

    tasks.forEach((ptr, taskId) => {
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
    });

    const execTime = performance.now() - startTime;

    workerScope.postMessage({
      type: 'STATE_UPDATE',
      tasks: updates,
      events,
      perf: { execTime, taskCount: tasks.size },
    });

    // Emit SystemOverheat when the worker manages too many concurrent tasks.
    if (tasks.size >= SYSTEM_OVERHEAT_TASK_COUNT) {
      workerScope.postMessage({
        type: 'STATE_UPDATE',
        tasks: {},
        events: [{ type: 'SystemOverheat', taskCount: tasks.size }],
        perf: { execTime: 0, taskCount: tasks.size },
      });
    }
  }, STEP_MS);
}

function stopSimulation() {
  if (simInterval) {
    clearInterval(simInterval);
    simInterval = null;
  }
  // Explicit cleanup – free every WASM allocation
  tasks.forEach((ptr) => {
    if (Module) Module.Task_Destroy(ptr);
  });
  tasks.clear();
}

export {};
