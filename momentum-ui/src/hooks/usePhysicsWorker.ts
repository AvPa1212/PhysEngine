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

// Simulated physics for when WASM is unavailable
function makeSimState(taskId: string, mass: number, t: number): TaskState {
  const seed = taskId.split('').reduce((a, c) => a + c.charCodeAt(0), 0);
  const phase = (seed % 100) / 100;
  return {
    stressX: Math.sin(t * 0.8 + phase * Math.PI * 2) * mass * 0.4,
    stressY: Math.cos(t * 0.6 + phase * Math.PI * 2) * mass * 0.3,
    stressZ: Math.sin(t * 0.4 + phase) * 0.2,
    entropy: 0.5 + Math.abs(Math.sin(t * 0.3 + phase)) * mass * 0.3,
    posX: Math.sin(t * 0.5 + phase * Math.PI * 2) * (0.5 + mass * 0.05),
    posY: Math.cos(t * 0.4 + phase * Math.PI * 2) * (0.4 + mass * 0.04),
    collapseProbability: 0.1 + Math.abs(Math.sin(t * 0.2 + phase)) * 0.4,
    stepCount: Math.floor(t * 60),
  };
}

export function usePhysicsWorker() {
  const workerRef = useRef<Worker | null>(null);
  const [isReady, setIsReady] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [taskStates, setTaskStates] = useState<Record<string, TaskState>>({});
  const eventBridgeRef = useRef(new EventBridge());
  const perfMonitorRef = useRef(new PerformanceMonitor());
  const serializeCallbacksRef = useRef(new Map<string, (state: string) => void>());

  // Simulated mode state
  const simModeRef = useRef(false);
  const simTasksRef = useRef<Map<string, number>>(new Map()); // taskId → mass
  const simTimeRef = useRef(0);
  const simIntervalRef = useRef<ReturnType<typeof setInterval> | null>(null);

  const startSimMode = useCallback(() => {
    if (simModeRef.current) return;
    simModeRef.current = true;
    setIsReady(true);
    simIntervalRef.current = setInterval(() => {
      simTimeRef.current += 1 / 60;
      const t = simTimeRef.current;
      const updates: Record<string, TaskState> = {};
      simTasksRef.current.forEach((mass, taskId) => {
        updates[taskId] = makeSimState(taskId, mass, t);
      });
      if (simTasksRef.current.size > 0) {
        setTaskStates(updates);
      }
    }, 1000 / 60);
  }, []);

  useEffect(() => {
    const eventBridge = eventBridgeRef.current;
    const baseUrl = import.meta.env.BASE_URL || '/';
    const webDistBase = `${baseUrl}web_dist/`;

    let worker: Worker;
    try {
      worker = new Worker(new URL('../workers/physicsWorker.ts', import.meta.url));
    } catch {
      // Workers unavailable – use sim mode immediately
      startSimMode();
      return;
    }
    workerRef.current = worker;

    // Fallback timeout: if WASM doesn't load within 4s, switch to sim mode
    const fallbackTimer = setTimeout(() => {
      if (!isReady && !simModeRef.current) {
        startSimMode();
      }
    }, 4000);

    worker.onmessage = (e: MessageEvent<any>) => {
      const { type, ...data } = e.data;
      switch (type) {
        case 'READY':
          clearTimeout(fallbackTimer);
          setIsReady(true);
          break;
        case 'STATE_UPDATE':
          setTaskStates(data.tasks);
          if (data.events) {
            data.events.forEach((evt: { type: string }) =>
              eventBridgeRef.current.emit(evt.type, evt)
            );
          }
          if (data.perf) perfMonitorRef.current.record(data.perf);
          break;
        case 'SERIALIZED': {
          const cb = serializeCallbacksRef.current.get(data.taskId);
          if (cb) { cb(data.state); serializeCallbacksRef.current.delete(data.taskId); }
          try { localStorage.setItem(`momentum_task_${data.taskId}`, data.state); } catch { }
          break;
        }
        case 'ERROR':
          clearTimeout(fallbackTimer);
          // WASM unavailable – fall back to sim mode instead of showing error
          startSimMode();
          break;
        default:
          break;
      }
    };

    worker.onerror = () => {
      clearTimeout(fallbackTimer);
      startSimMode();
    };

    worker.postMessage({
      type: 'INIT',
      wasmPath: `${webDistBase}MomentumCore.js`,
      wasmDir: webDistBase,
    });

    return () => {
      clearTimeout(fallbackTimer);
      worker.postMessage({ type: 'STOP' });
      worker.terminate();
      workerRef.current = null;
      eventBridge.clear();
      if (simIntervalRef.current) clearInterval(simIntervalRef.current);
    };
  // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  const createTask = useCallback((taskId: string, options: Record<string, unknown> = {}) => {
    if (simModeRef.current) {
      simTasksRef.current.set(taskId, (options.mass as number) ?? 1);
      return;
    }
    workerRef.current?.postMessage({ type: 'CREATE_TASK', taskId, ...options });
  }, []);

  const destroyTask = useCallback((taskId: string) => {
    if (simModeRef.current) { simTasksRef.current.delete(taskId); return; }
    workerRef.current?.postMessage({ type: 'DESTROY_TASK', taskId });
  }, []);

  const applyForce = useCallback((taskId: string, fx: number, fy: number, fz: number) => {
    if (simModeRef.current) return;
    workerRef.current?.postMessage({ type: 'APPLY_FORCE', taskId, fx, fy, fz });
  }, []);

  const setMass = useCallback((taskId: string, mass: number) => {
    if (simModeRef.current) { simTasksRef.current.set(taskId, mass); return; }
    workerRef.current?.postMessage({ type: 'SET_MASS', taskId, mass });
  }, []);

  const collapse = useCallback((taskId: string) => {
    if (simModeRef.current) return;
    workerRef.current?.postMessage({ type: 'COLLAPSE', taskId });
  }, []);

  const serialize = useCallback((taskId: string) => {
    return new Promise<string>((resolve, reject) => {
      if (simModeRef.current) { resolve('{}'); return; }
      if (!workerRef.current) { reject(new Error('Physics worker is not ready')); return; }
      serializeCallbacksRef.current.set(taskId, resolve);
      workerRef.current.postMessage({ type: 'SERIALIZE', taskId });
    });
  }, []);

  const deserialize = useCallback((taskId: string, state: string) => {
    if (simModeRef.current) return;
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
