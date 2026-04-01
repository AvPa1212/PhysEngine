/**
 * useMomentum.ts
 *
 * Custom React hook that asynchronously loads the Emscripten-compiled
 * WebAssembly physics engine and exposes it to consuming components.
 *
 * Loading strategy:
 *  1. A <script> tag is injected into <body> pointing at the Emscripten
 *     JavaScript glue file (MomentumCore.js).  The glue file exposes a
 *     global `window.PhysEngine` factory function.
 *  2. Once the script has loaded, the factory is called with a locateFile
 *     callback that resolves the `.wasm` binary to the correct URL.
 *  3. When the WASM module has fully compiled and instantiated, the resolved
 *     Module object is stored in state and `isReady` is set to true.
 *
 * Usage:
 * @example
 *   const { engine, isReady } = useMomentum();
 *   if (!isReady) return <Spinner />;
 *   const task = engine.Task_Create();
 *
 * @returns {{ engine: object | null, isReady: boolean }}
 *   - engine:  The Emscripten module instance once loaded, or null while loading.
 *   - isReady: true when the WASM binary has fully initialised and the
 *              engine is safe to use.
 */
import { useState, useEffect } from 'react';

export function useMomentum() {
  /** Emscripten module object; null until the WASM binary is ready. */
  const [engine, setEngine] = useState(null);

  /** Becomes true once the WASM module has fully initialised. */
  const [isReady, setIsReady] = useState(false);

  useEffect(() => {
    // Dynamically insert the Emscripten glue script so we don't block the
    // initial page render while the ~200 KB WASM binary downloads.
    // Loading the generated Emscripten glue code
    const script = document.createElement('script');
    script.src = '/web_dist/MomentumCore.js';
    script.async = true;

    script.onload = async () => {
      // The glue script registers window.PhysEngine as an async factory.
      // Calling it triggers WASM compilation and memory initialisation.
      // The locateFile callback tells the factory where to find the .wasm
      // binary relative to the web server root.
      const Module = await window.PhysEngine({
        locateFile: (path) => path.endsWith('.wasm') ? `/web_dist/${path}` : path
      });

      // Store the fully initialised module so React components can call
      // the exported C++ functions (Task_Create, Engine_UpdateChaos, etc.).
      setEngine(Module);
      setIsReady(true);
    };

    // Append to <body> to trigger the browser's script download.
    document.body.appendChild(script);

    // No cleanup needed: the WASM module persists for the page lifetime and
    // the script tag is harmless to leave in the DOM.
  }, []); // Empty dependency array — run once on mount.

  return { engine, isReady };
}