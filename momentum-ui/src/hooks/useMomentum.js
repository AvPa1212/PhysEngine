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
    };
  }, []);

  return { engine, isReady, error };
}