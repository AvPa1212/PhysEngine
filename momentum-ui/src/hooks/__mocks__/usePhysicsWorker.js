/**
 * Jest mock for usePhysicsWorker.
 *
 * Web Workers and import.meta.url are unavailable in the Jest/jsdom test
 * environment.  This mock returns the same shape as the real hook but stays
 * in the "loading" state so existing tests (e.g. "LOADING QUANTUM CORE")
 * continue to pass.
 */
export function usePhysicsWorker() {
  return {
    isReady: false,
    error: null,
    taskStates: {},
    createTask: () => {},
    destroyTask: () => {},
    applyForce: () => {},
    setMass: () => {},
    collapse: () => {},
    serialize: () => {},
    deserialize: () => {},
    eventBridge: null,
    perfMonitor: null,
  };
}
