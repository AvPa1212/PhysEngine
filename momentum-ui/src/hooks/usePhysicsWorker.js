import { useState, useEffect, useRef, useCallback } from 'react';
import { EventBridge } from '../services/EventBridge';
import { PerformanceMonitor } from '../services/PerformanceMonitor';

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
  const workerRef = useRef(null);
  const [isReady, setIsReady] = useState(false);
  const [error, setError] = useState(null);
  const [taskStates, setTaskStates] = useState({});
  const eventBridgeRef = useRef(new EventBridge());
  const perfMonitorRef = useRef(new PerformanceMonitor());

  // Pending serialisation callbacks keyed by taskId.
  const serializeCallbacksRef = useRef(new Map());

  useEffect(() => {
    let worker;
    try {
      worker = new Worker(
        new URL('../workers/physicsWorker.js', import.meta.url)
      );
    } catch {
      // Workers not available (e.g. test / SSR environment) – stay in loading.
      return;
    }
    workerRef.current = worker;

    worker.onmessage = (e) => {
      const { type, ...data } = e.data;

      switch (type) {
        case 'READY':
          setIsReady(true);
          break;

        case 'STATE_UPDATE':
          setTaskStates(data.tasks);
          // Forward engine events to the EventBridge
          if (data.events) {
            data.events.forEach((evt) =>
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
      wasmPath: '/web_dist/MomentumCore.js',
      wasmDir: '/web_dist/',
    });

    return () => {
      worker.postMessage({ type: 'STOP' });
      worker.terminate();
      workerRef.current = null;
      eventBridgeRef.current.clear();
      perfMonitorRef.current.reset();
    };
  }, []);

  // ── Public API ──────────────────────────────────────────────────────────────

  const createTask = useCallback((taskId, options = {}) => {
    workerRef.current?.postMessage({ type: 'CREATE_TASK', taskId, ...options });
  }, []);

  const destroyTask = useCallback((taskId) => {
    workerRef.current?.postMessage({ type: 'DESTROY_TASK', taskId });
  }, []);

  const applyForce = useCallback((taskId, fx, fy, fz) => {
    workerRef.current?.postMessage({
      type: 'APPLY_FORCE',
      taskId,
      fx,
      fy,
      fz,
    });
  }, []);

  const setMass = useCallback((taskId, mass) => {
    workerRef.current?.postMessage({ type: 'SET_MASS', taskId, mass });
  }, []);

  const collapse = useCallback((taskId) => {
    workerRef.current?.postMessage({ type: 'COLLAPSE', taskId });
  }, []);

  const serialize = useCallback((taskId) => {
    workerRef.current?.postMessage({ type: 'SERIALIZE', taskId });
  }, []);

  const deserialize = useCallback((taskId, state) => {
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
