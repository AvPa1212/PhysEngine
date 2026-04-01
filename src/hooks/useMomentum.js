import { useState, useEffect } from 'react';

export function useMomentum() {
  const [engine, setEngine] = useState(null);
  const [isReady, setIsReady] = useState(false);

  useEffect(() => {
    // Loading the generated Emscripten glue code
    const script = document.createElement('script');
    script.src = '/web_dist/MomentumCore.js';
    script.async = true;
    script.onload = async () => {
      const Module = await window.PhysEngine({
        locateFile: (path) => path.endsWith('.wasm') ? `/web_dist/${path}` : path
      });
      setEngine(Module);
      setIsReady(true);
    };
    document.body.appendChild(script);
  }, []);

  return { engine, isReady };
}