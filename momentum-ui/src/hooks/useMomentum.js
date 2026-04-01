import { useState, useEffect } from 'react';

export function useMomentum() {
  const [engine, setEngine] = useState(null);
  const [isReady, setIsReady] = useState(false);
  const [error, setError] = useState(null);

  useEffect(() => {
    let cancelled = false;

    // Loading the generated Emscripten glue code
    const script = document.createElement('script');
    script.src = '/web_dist/MomentumCore.js';
    script.async = true;
    script.onload = async () => {
      // Guard against StrictMode double-mount: if the effect already cleaned up,
      // do nothing and let the second mount handle initialisation.
      if (cancelled) return;
      try {
        const Module = await window.PhysEngine({
          locateFile: (path) => path.endsWith('.wasm') ? `/web_dist/${path}` : path
        });
        if (!cancelled) {
          setEngine(Module);
          setIsReady(true);
        }
      } catch (err) {
        if (!cancelled) {
          setError(err?.message || 'Failed to initialize physics engine.');
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

  return { engine, isReady, error };
}