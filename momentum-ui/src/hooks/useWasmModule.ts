import { useEffect, useState } from 'react';

type WrappedModule = {
  _raw: unknown;
  Task_Create: () => number;
  Task_CreateWithParams: (mass: number, deadlineTime: number, urgencyConstant: number) => number;
  Task_Destroy: (taskPtr: number) => void;
  Task_GetPositionX: (taskPtr: number) => number;
  Task_GetPositionY: (taskPtr: number) => number;
  Task_GetVelocityX: (taskPtr: number) => number;
  Task_GetVelocityY: (taskPtr: number) => number;
  Task_GetAccelerationX: (taskPtr: number) => number;
  Task_GetAccelerationY: (taskPtr: number) => number;
  Task_GetMass: (taskPtr: number) => number;
  Task_SetPosition: (taskPtr: number, x: number, y: number) => void;
  Task_SetVelocity: (taskPtr: number, vx: number, vy: number) => void;
  Task_SetMass: (taskPtr: number, mass: number) => void;
  Engine_IntegrateClassical: (taskPtr: number) => void;
  State_Serialize: (taskPtr: number) => string;
  State_Deserialize: (taskPtr: number, state: string) => void;
  // Energy functions
  Energy_GetKinetic: (taskPtr: number) => number;
  Energy_GetPotential: (taskPtr: number) => number;
  Energy_GetTotal: (taskPtr: number) => number;
  System_GetTotalEnergy: (enginePtr: number) => number;
  Energy_Inject: (taskPtr: number, amount: number) => void;
  System_EnableDamping: (enginePtr: number, coefficient: number) => void;
  System_DisableDamping: (enginePtr: number) => void;
};

type EmscriptenModule = {
  cwrap: (name: string, returnType: string | null, argTypes: string[]) => (...args: unknown[]) => unknown;
};

type FactoryFn = (moduleArg?: {
  locateFile: (path: string) => string;
}) => Promise<EmscriptenModule>;

export function useWasmModule() {
  const [module, setModule] = useState<WrappedModule | null>(null);
  const [error, setError] = useState<string | null>(null);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    let cancelled = false;
    const baseUrl = import.meta.env.BASE_URL || '/';
    const webDistBase = `${baseUrl}web_dist/`;

    const script = document.createElement('script');
    script.src = `${webDistBase}MomentumCore.js`;
    script.async = true;

    script.onload = async () => {
      if (cancelled) return;

      try {
        const resolveFactory = async (): Promise<FactoryFn> => {
          const globals = globalThis as typeof globalThis & {
            PhysEngine?: unknown;
            Module?: unknown;
            createModule?: unknown;
            MomentumCore?: unknown;
          };

          const candidates = [
            globals.PhysEngine,
            globals.Module,
            globals.createModule,
            globals.MomentumCore,
          ];

          const globalFactory =
            candidates.find((factory) => typeof factory === 'function') ||
            candidates
              .map((candidate) => (candidate as { default?: unknown })?.default)
              .find((factory) => typeof factory === 'function') ||
            null;

          if (globalFactory) {
            return globalFactory as FactoryFn;
          }

          const response = await fetch(`${webDistBase}MomentumCore.js`, { cache: 'no-store' });
          const source = await response.text();
          const evaluatedFactory = new Function(
            `${source}\nreturn (typeof PhysEngine === "function" && PhysEngine) || (typeof Module === "function" && Module) || (typeof createModule === "function" && createModule) || (typeof MomentumCore === "function" && MomentumCore) || null;`
          )() as FactoryFn | null;

          if (typeof evaluatedFactory === 'function') {
            return evaluatedFactory;
          }

          throw new Error(
            `MomentumCore loaded, but no callable Emscripten factory was found. HTTP ${response.status} content-type=${response.headers.get('content-type') || 'unknown'}`
          );
        };

        const engineFactory = await resolveFactory();

        const rawModule = await engineFactory({
          locateFile: (path) => (path.endsWith('.wasm') ? `${webDistBase}${path}` : path),
        });

        if (cancelled) return;

        const wrapped: WrappedModule = {
          _raw: rawModule,
          Task_Create: rawModule.cwrap('Task_Create', 'number', []) as () => number,
          Task_CreateWithParams: rawModule.cwrap('Task_CreateWithParams', 'number', ['number', 'number', 'number']) as (
            mass: number,
            deadlineTime: number,
            urgencyConstant: number
          ) => number,
          Task_Destroy: rawModule.cwrap('Task_Destroy', null, ['number']) as (taskPtr: number) => void,
          Task_GetPositionX: rawModule.cwrap('Task_GetPositionX', 'number', ['number']) as (taskPtr: number) => number,
          Task_GetPositionY: rawModule.cwrap('Task_GetPositionY', 'number', ['number']) as (taskPtr: number) => number,
          Task_GetVelocityX: rawModule.cwrap('Task_GetVelocityX', 'number', ['number']) as (taskPtr: number) => number,
          Task_GetVelocityY: rawModule.cwrap('Task_GetVelocityY', 'number', ['number']) as (taskPtr: number) => number,
          Task_GetAccelerationX: rawModule.cwrap('Task_GetAccelerationX', 'number', ['number']) as (taskPtr: number) => number,
          Task_GetAccelerationY: rawModule.cwrap('Task_GetAccelerationY', 'number', ['number']) as (taskPtr: number) => number,
          Task_GetMass: rawModule.cwrap('Task_GetMass', 'number', ['number']) as (taskPtr: number) => number,
          Task_SetPosition: rawModule.cwrap('Task_SetPosition', null, ['number', 'number', 'number']) as (
            taskPtr: number,
            x: number,
            y: number
          ) => void,
          Task_SetVelocity: rawModule.cwrap('Task_SetVelocity', null, ['number', 'number', 'number']) as (
            taskPtr: number,
            vx: number,
            vy: number
          ) => void,
          Task_SetMass: rawModule.cwrap('Task_SetMass', null, ['number', 'number']) as (taskPtr: number, mass: number) => void,
          Engine_IntegrateClassical: rawModule.cwrap('Engine_IntegrateClassical', null, ['number']) as (
            taskPtr: number
          ) => void,
          State_Serialize: rawModule.cwrap('State_Serialize', 'string', ['number']) as (taskPtr: number) => string,
          State_Deserialize: rawModule.cwrap('State_Deserialize', null, ['number', 'string']) as (
            taskPtr: number,
            state: string
          ) => void,
          Energy_GetKinetic: rawModule.cwrap('Energy_GetKinetic', 'number', ['number']) as (taskPtr: number) => number,
          Energy_GetPotential: rawModule.cwrap('Energy_GetPotential', 'number', ['number']) as (taskPtr: number) => number,
          Energy_GetTotal: rawModule.cwrap('Energy_GetTotal', 'number', ['number']) as (taskPtr: number) => number,
          System_GetTotalEnergy: rawModule.cwrap('System_GetTotalEnergy', 'number', ['number']) as (enginePtr: number) => number,
          Energy_Inject: rawModule.cwrap('Energy_Inject', null, ['number', 'number']) as (taskPtr: number, amount: number) => void,
          System_EnableDamping: rawModule.cwrap('System_EnableDamping', null, ['number', 'number']) as (enginePtr: number, coefficient: number) => void,
          System_DisableDamping: rawModule.cwrap('System_DisableDamping', null, ['number']) as (enginePtr: number) => void,
        };

        setModule(wrapped);
        setLoading(false);
      } catch (err) {
        if (!cancelled) {
          setError(err instanceof Error ? err.message : 'Failed to initialize WebAssembly module.');
          setLoading(false);
        }
      }
    };

    script.onerror = () => {
      if (!cancelled) {
        setError('Failed to load MomentumCore.js. Ensure the WASM build has been run.');
        setLoading(false);
      }
    };

    document.body.appendChild(script);

    return () => {
      cancelled = true;
      script.onload = null;
      script.onerror = null;
      if (document.body.contains(script)) {
        document.body.removeChild(script);
      }
    };
  }, []);

  return { module, error, loading };
}