import { useState, useEffect, useRef, useCallback } from 'react';
import { EventBridge } from '../services/EventBridge';
import { PerformanceMonitor } from '../services/PerformanceMonitor';

type TaskState = {
  stressX: number;
  stressY: number;
  stressZ: number;
  entropy: number;
  posX: number;
  posY: number;
  collapseProbability: number;
  stepCount: number;
};

/**
 * usePhysicsWorker – React hook that moves the WASM physics simulation into a
 * dedicated Web Worker and provides a message-passing API to the React layer.
 *
 * Returns the same loading / error state shape as the original `useMomentum`
 * hook so the rest of the app can remain largely unchanged.
 *
 * Memory contract:
 *   React component mounts  → call createTask(taskId, opts)
 *   React component unmounts→ call destroyTask(taskId)
 *   This guarantees a 1:1 mapping of C++ pointers to React lifecycle.
 */
export function usePhysicsWorker() {
  const workerRef = useRef<Worker | null>(null);
  const [isReady, setIsReady] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [taskStates, setTaskStates] = useState<Record<string, TaskState>>({});
  const eventBridgeRef = useRef(new EventBridge());
  const perfMonitorRef = useRef(new PerformanceMonitor());

  // Pending serialisation callbacks keyed by taskId.
  const serializeCallbacksRef = useRef(new Map<string, (state: string) => void>());

  useEffect(() => {
    const eventBridge = eventBridgeRef.current;
    const perfMonitor = perfMonitorRef.current;
    const baseUrl = import.meta.env.BASE_URL || '/';
    const webDistBase = `${baseUrl}web_dist/`;
    let worker: Worker;
    try {
      worker = new Worker(
        new URL('../workers/physicsWorker.ts', import.meta.url)
      );
    } catch {
      // Workers not available (e.g. test / SSR environment) – stay in loading.
      return;
    }
    workerRef.current = worker;

    worker.onmessage = (e: MessageEvent<any>) => {
      const { type, ...data } = e.data;

      switch (type) {
        case 'READY':
          setIsReady(true);
          break;

        case 'STATE_UPDATE':
          setTaskStates(data.tasks);
          // Forward engine events to the EventBridge
          if (data.events) {
            data.events.forEach((evt: { type: string }) =>
              eventBridgeRef.current.emit(evt.type, evt)
            );
          }
          // Record telemetry
          if (data.perf) {
            perfMonitorRef.current.record(data.perf);
          }
          break;

        case 'SERIALIZED': {
          // Resolve any pending serialisation promise
          const cb = serializeCallbacksRef.current.get(data.taskId);
          if (cb) {
            cb(data.state);
            serializeCallbacksRef.current.delete(data.taskId);
          }
          // Also store in localStorage for persistence
          try {
            localStorage.setItem(
              `momentum_task_${data.taskId}`,
              data.state
            );
          } catch {
            /* localStorage may be unavailable */
          }
          break;
        }

        case 'ERROR':
          setError(data.message);
          break;

        default:
          break;
      }
    };

    worker.onerror = (err) => {
      setError(err?.message || 'Physics worker error');
    };

    // Boot the worker – tell it where to find the WASM artefacts.
    worker.postMessage({
      type: 'INIT',
      wasmPath: `${webDistBase}MomentumCore.js`,
      wasmDir: webDistBase,
    });

    return () => {
      worker.postMessage({ type: 'STOP' });
      worker.terminate();
      workerRef.current = null;
      eventBridge.clear();
      perfMonitor.reset();
    };
  }, []);

  // ── Public API ──────────────────────────────────────────────────────────────

  const createTask = useCallback((taskId: string, options: Record<string, unknown> = {}) => {
    workerRef.current?.postMessage({ type: 'CREATE_TASK', taskId, ...options });
  }, []);

  const destroyTask = useCallback((taskId: string) => {
    workerRef.current?.postMessage({ type: 'DESTROY_TASK', taskId });
  }, []);

  const applyForce = useCallback((taskId: string, fx: number, fy: number, fz: number) => {
    workerRef.current?.postMessage({
      type: 'APPLY_FORCE',
      taskId,
      fx,
      fy,
      fz,
    });
  }, []);

  const setMass = useCallback((taskId: string, mass: number) => {
    workerRef.current?.postMessage({ type: 'SET_MASS', taskId, mass });
  }, []);

  const collapse = useCallback((taskId: string) => {
    workerRef.current?.postMessage({ type: 'COLLAPSE', taskId });
  }, []);

  const serialize = useCallback((taskId: string) => {
    return new Promise<string>((resolve, reject) => {
      if (!workerRef.current) {
        reject(new Error('Physics worker is not ready'));
        return;
      }
      serializeCallbacksRef.current.set(taskId, resolve);
      workerRef.current.postMessage({ type: 'SERIALIZE', taskId });
    });
  }, []);

  const deserialize = useCallback((taskId: string, state: string) => {
    workerRef.current?.postMessage({ type: 'DESERIALIZE', taskId, state });
  }, []);

  return {
    isReady,
    error,
    taskStates,
    createTask,
    destroyTask,
    applyForce,
    setMass,
    collapse,
    serialize,
    deserialize,
    eventBridge: eventBridgeRef.current,
    perfMonitor: perfMonitorRef.current,
  };
}
