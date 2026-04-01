import { useState, useEffect } from 'react';

/**
 * useWasmModule – React hook that loads the MomentumCore WebAssembly module
 * and wraps the C bridge functions via Module.cwrap.
 *
 * Returns { module, error, loading } where:
 *   - module: object with wrapped bridge functions (or null while loading)
 *   - error:  string error message on failure (or null on success)
 *   - loading: true while the module is being fetched/initialised
 *
 * Requirements: 10.1, 10.2, 10.6
 */
export function useWasmModule() {
  const [module, setModule] = useState(null);
  const [error, setError] = useState(null);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    let cancelled = false;

    const script = document.createElement('script');
    script.src = '/MomentumCore.js';
    script.async = true;

    script.onload = async () => {
      if (cancelled) return;
      try {
        const Module = await window.PhysEngine({
          locateFile: (path) =>
            path.endsWith('.wasm') ? `/${path}` : path,
        });

        if (cancelled) return;

        // Wrap the C bridge functions so callers don't need to know about
        // Emscripten's calling conventions.
        const wrapped = {
          _raw: Module,

          // Task lifecycle
          Task_Create: Module.cwrap('Task_Create', 'number', []),
          Task_Destroy: Module.cwrap('Task_Destroy', null, ['number']),

          // State queries
          Task_GetPositionX: Module.cwrap('Task_GetPositionX', 'number', ['number']),
          Task_GetPositionY: Module.cwrap('Task_GetPositionY', 'number', ['number']),
          Task_GetVelocityX: Module.cwrap('Task_GetVelocityX', 'number', ['number']),
          Task_GetVelocityY: Module.cwrap('Task_GetVelocityY', 'number', ['number']),
          Task_GetMass: Module.cwrap('Task_GetMass', 'number', ['number']),

          // State mutations
          Task_SetPosition: Module.cwrap('Task_SetPosition', null, ['number', 'number', 'number']),
          Task_SetVelocity: Module.cwrap('Task_SetVelocity', null, ['number', 'number', 'number']),
          Task_SetMass: Module.cwrap('Task_SetMass', null, ['number', 'number']),

          // Physics engine
          Engine_IntegrateClassical: Module.cwrap('Engine_IntegrateClassical', null, ['number']),

          // Serialization
          State_Serialize: Module.cwrap('State_Serialize', 'string', ['number']),
          State_Deserialize: Module.cwrap('State_Deserialize', null, ['number', 'string']),
        };

        setModule(wrapped);
        setLoading(false);
      } catch (err) {
        if (!cancelled) {
          setError(
            err instanceof Error
              ? err.message
              : 'Failed to initialize WebAssembly module.'
          );
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
