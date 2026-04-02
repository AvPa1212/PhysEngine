import { useState, useEffect, useCallback } from 'react';

type TaskEnergyState = {
  kineticEnergy: number;
  potentialEnergy: number;
  totalEnergy: number;
};

export function useMomentum() {
  const [engine, setEngine] = useState<unknown>(null);
  const [isReady, setIsReady] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [taskEnergies, setTaskEnergies] = useState<Record<string, TaskEnergyState>>({});
  const [systemEnergy, setSystemEnergy] = useState<number>(0);
  const [isDampingEnabled, setIsDampingEnabled] = useState<boolean>(false);
  const [dampingCoefficient, setDampingCoefficient] = useState<number>(0.1);

  useEffect(() => {
    let cancelled = false;
    const baseUrl = import.meta.env.BASE_URL || '/';
    const webDistBase = `${baseUrl}web_dist/`;

    // Loading the generated Emscripten glue code
    const script = document.createElement('script');
    script.src = `${webDistBase}MomentumCore.js`;
    script.async = true;
    script.onload = async () => {
      // Guard against StrictMode double-mount: if the effect already cleaned up,
      // do nothing and let the second mount handle initialisation.
      if (cancelled) return;
      try {
        const resolveFactory = async (): Promise<((moduleArg?: unknown) => Promise<unknown>)> => {
          const globals = globalThis as any;
          const candidates = [
            globals.PhysEngine,
            globals.Module,
            globals.createModule,
            globals.MomentumCore,
          ];
          const globalFactory =
            candidates.find((factory) => typeof factory === 'function') ||
            candidates
              .map((candidate) => candidate?.default)
              .find((factory) => typeof factory === 'function') ||
            null;

          if (globalFactory) {
            return globalFactory;
          }

          // Some hosts execute the script without attaching top-level vars to globalThis.
          // Fallback: evaluate the downloaded script and return its factory symbol.
          const response = await fetch(`${webDistBase}MomentumCore.js`, { cache: 'no-store' });
          const source = await response.text();
          const evaluatedFactory = new Function(
            `${source}\nreturn (typeof PhysEngine === "function" && PhysEngine) || (typeof Module === "function" && Module) || (typeof createModule === "function" && createModule) || (typeof MomentumCore === "function" && MomentumCore) || null;`
          )();

          if (typeof evaluatedFactory === 'function') {
            return evaluatedFactory;
          }

          throw new Error(
            `MomentumCore loaded, but no callable Emscripten factory was found. HTTP ${response.status} content-type=${response.headers.get('content-type') || 'unknown'}`
          );
        };

        const engineFactory = await resolveFactory();

        const Module = await engineFactory({
          locateFile: (path) => path.endsWith('.wasm') ? `${webDistBase}${path}` : path
        });
        if (!cancelled) {
          setEngine(Module);
          setIsReady(true);
        }
      } catch (err: unknown) {
        if (!cancelled) {
          const message =
            err instanceof Error
              ? err.message
              : 'Failed to initialize physics engine.';
          setError(message);
        }
      }
    };
    script.onerror = () => {
      if (!cancelled) {
        setError('Failed to load MomentumCore.js. Run the WASM build first.');
      }
    };
    document.body.appendChild(script);

    return () => {
      cancelled = true;
      // Null the handlers so a pending async onload callback cannot fire
      // state updates after this effect instance has been torn down.
      script.onload = null;
      script.onerror = null;
      // Remove the injected script to avoid duplicate elements during
      // React StrictMode's intentional mount → unmount → remount cycle.
      if (document.body.contains(script)) {
        document.body.removeChild(script);
      }
    };
  }, []);

  // Query energy values for a task by its pointer
  const getTaskEnergy = useCallback((taskPtr: number): TaskEnergyState => {
    const mod = engine as any;
    if (!mod || !isReady) {
      return { kineticEnergy: 0, potentialEnergy: 0, totalEnergy: 0 };
    }
    return {
      kineticEnergy: typeof mod.Energy_GetKinetic === 'function' ? mod.Energy_GetKinetic(taskPtr) : 0,
      potentialEnergy: typeof mod.Energy_GetPotential === 'function' ? mod.Energy_GetPotential(taskPtr) : 0,
      totalEnergy: typeof mod.Energy_GetTotal === 'function' ? mod.Energy_GetTotal(taskPtr) : 0,
    };
  }, [engine, isReady]);

  // Update energy state for a set of task pointers (keyed by taskId)
  const refreshTaskEnergies = useCallback((taskPtrs: Record<string, number>) => {
    const mod = engine as any;
    if (!mod || !isReady) return;
    const updated: Record<string, TaskEnergyState> = {};
    for (const [taskId, ptr] of Object.entries(taskPtrs)) {
      updated[taskId] = {
        kineticEnergy: typeof mod.Energy_GetKinetic === 'function' ? mod.Energy_GetKinetic(ptr) : 0,
        potentialEnergy: typeof mod.Energy_GetPotential === 'function' ? mod.Energy_GetPotential(ptr) : 0,
        totalEnergy: typeof mod.Energy_GetTotal === 'function' ? mod.Energy_GetTotal(ptr) : 0,
      };
    }
    setTaskEnergies(updated);
  }, [engine, isReady]);

  // Query total system energy
  const getSystemEnergy = useCallback((enginePtr: number): number => {
    const mod = engine as any;
    if (!mod || !isReady) return 0;
    if (typeof mod.System_GetTotalEnergy === 'function') {
      const val = mod.System_GetTotalEnergy(enginePtr);
      setSystemEnergy(val);
      return val;
    }
    return 0;
  }, [engine, isReady]);

  // Inject energy into a task
  const injectEnergy = useCallback((taskPtr: number, energyAmount: number): void => {
    const mod = engine as any;
    if (!mod || !isReady) return;
    if (typeof mod.Energy_Inject === 'function') {
      mod.Energy_Inject(taskPtr, energyAmount);
    }
  }, [engine, isReady]);

  // Enable damping on the engine
  const enableDamping = useCallback((enginePtr: number, coefficient: number): void => {
    const mod = engine as any;
    if (!mod || !isReady) return;
    if (typeof mod.System_EnableDamping === 'function') {
      mod.System_EnableDamping(enginePtr, coefficient);
    }
    setIsDampingEnabled(true);
    setDampingCoefficient(coefficient);
  }, [engine, isReady]);

  // Disable damping on the engine
  const disableDamping = useCallback((enginePtr: number): void => {
    const mod = engine as any;
    if (!mod || !isReady) return;
    if (typeof mod.System_DisableDamping === 'function') {
      mod.System_DisableDamping(enginePtr);
    }
    setIsDampingEnabled(false);
  }, [engine, isReady]);

  return { engine, isReady, error, taskEnergies, systemEnergy, isDampingEnabled, dampingCoefficient, getTaskEnergy, refreshTaskEnergies, getSystemEnergy, injectEnergy, enableDamping, disableDamping };
}