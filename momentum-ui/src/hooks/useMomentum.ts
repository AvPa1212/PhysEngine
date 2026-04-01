import { useState, useEffect } from 'react';

export function useMomentum() {
  const [engine, setEngine] = useState<unknown>(null);
  const [isReady, setIsReady] = useState(false);
  const [error, setError] = useState<string | null>(null);

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
        const Module = await window.PhysEngine({
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

  return { engine, isReady, error };
}