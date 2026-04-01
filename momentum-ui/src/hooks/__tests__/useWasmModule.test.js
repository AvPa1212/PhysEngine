/**
 * Unit tests for useWasmModule hook.
 * Requirements: 10.6
 *
 * Strategy:
 *  - Mock script injection so we can control when onload / onerror fires.
 *  - Mock window.PhysEngine to simulate successful / failed WASM init.
 *  - Use @testing-library/react renderHook for clean hook lifecycle testing.
 */

import { renderHook, act, waitFor } from '@testing-library/react';
import { useWasmModule } from '../useWasmModule';

// ─── helpers ────────────────────────────────────────────────────────────────

/** Build a minimal fake Emscripten Module with cwrap support. */
function makeFakeModule(overrides = {}) {
  return {
    cwrap: vi.fn((name) => vi.fn().mockReturnValue(`result_of_${name}`)),
    ...overrides,
  };
}

// ─── setup / teardown ───────────────────────────────────────────────────────

let appendedScript = null;
let originalAppendChild;
let originalRemoveChild;
let originalContains;

beforeEach(() => {
  appendedScript = null;

  // Intercept document.body.appendChild so we can grab the injected <script>
  // and fire its callbacks manually.
  originalAppendChild = document.body.appendChild.bind(document.body);
  originalRemoveChild = document.body.removeChild.bind(document.body);
  originalContains = document.body.contains.bind(document.body);

  vi.spyOn(document.body, 'appendChild').mockImplementation((node) => {
    if (node.tagName === 'SCRIPT') {
      appendedScript = node;
      return node;
    }
    return originalAppendChild(node);
  });

  vi.spyOn(document.body, 'removeChild').mockImplementation((node) => {
    if (node === appendedScript) {
      appendedScript = null;
      return node;
    }
    return originalRemoveChild(node);
  });

  vi.spyOn(document.body, 'contains').mockImplementation((node) => {
    if (node === appendedScript) return appendedScript !== null;
    return originalContains(node);
  });
});

afterEach(() => {
  vi.restoreAllMocks();
  delete window.PhysEngine;
  appendedScript = null;
});

// ─── tests ──────────────────────────────────────────────────────────────────

describe('useWasmModule', () => {
  it('starts in loading state with no module or error', () => {
    const { result } = renderHook(() => useWasmModule());

    expect(result.current.loading).toBe(true);
    expect(result.current.module).toBeNull();
    expect(result.current.error).toBeNull();
  });

  it('injects a <script> pointing to /MomentumCore.js on mount', () => {
    renderHook(() => useWasmModule());

    expect(appendedScript).not.toBeNull();
    expect(appendedScript.src).toContain('/MomentumCore.js');
  });

  it('returns module object with wrapped functions on successful load', async () => {
    const fakeModule = makeFakeModule();
    window.PhysEngine = vi.fn().mockResolvedValue(fakeModule);

    const { result } = renderHook(() => useWasmModule());

    // Trigger the script's onload callback
    await act(async () => {
      appendedScript.onload();
    });

    await waitFor(() => expect(result.current.loading).toBe(false));

    expect(result.current.error).toBeNull();
    expect(result.current.module).not.toBeNull();

    // Verify key bridge functions are wrapped
    const mod = result.current.module;
    expect(typeof mod.Task_Create).toBe('function');
    expect(typeof mod.Task_Destroy).toBe('function');
    expect(typeof mod.Engine_IntegrateClassical).toBe('function');
    expect(typeof mod.State_Serialize).toBe('function');
    expect(typeof mod.State_Deserialize).toBe('function');
  });

  it('exposes the raw Module as _raw on the returned object', async () => {
    const fakeModule = makeFakeModule();
    window.PhysEngine = vi.fn().mockResolvedValue(fakeModule);

    const { result } = renderHook(() => useWasmModule());

    await act(async () => {
      appendedScript.onload();
    });

    await waitFor(() => expect(result.current.loading).toBe(false));

    expect(result.current.module._raw).toBe(fakeModule);
  });

  it('sets error state when script fails to load', async () => {
    const { result } = renderHook(() => useWasmModule());

    await act(async () => {
      appendedScript.onerror();
    });

    await waitFor(() => expect(result.current.loading).toBe(false));

    expect(result.current.module).toBeNull();
    expect(result.current.error).toMatch(/MomentumCore\.js/i);
  });

  it('sets error state when PhysEngine() rejects', async () => {
    window.PhysEngine = vi.fn().mockRejectedValue(new Error('WASM init failed'));

    const { result } = renderHook(() => useWasmModule());

    await act(async () => {
      appendedScript.onload();
    });

    await waitFor(() => expect(result.current.loading).toBe(false));

    expect(result.current.module).toBeNull();
    expect(result.current.error).toBe('WASM init failed');
  });

  it('sets a generic error message when PhysEngine throws a non-Error value', async () => {
    window.PhysEngine = vi.fn().mockRejectedValue('string error');

    const { result } = renderHook(() => useWasmModule());

    await act(async () => {
      appendedScript.onload();
    });

    await waitFor(() => expect(result.current.loading).toBe(false));

    expect(result.current.error).toBeTruthy();
    expect(typeof result.current.error).toBe('string');
  });

  it('removes the injected script on unmount', async () => {
    const fakeModule = makeFakeModule();
    window.PhysEngine = vi.fn().mockResolvedValue(fakeModule);

    const { unmount } = renderHook(() => useWasmModule());

    await act(async () => {
      appendedScript.onload();
    });

    unmount();

    // removeChild should have been called for the script element
    expect(document.body.removeChild).toHaveBeenCalled();
  });

  it('does not update state after unmount (cancelled flag)', async () => {
    // PhysEngine resolves after a delay so we can unmount first
    let resolvePhysEngine;
    window.PhysEngine = vi.fn(
      () => new Promise((resolve) => { resolvePhysEngine = resolve; })
    );

    const { result, unmount } = renderHook(() => useWasmModule());

    // Trigger onload but don't resolve PhysEngine yet
    act(() => {
      appendedScript.onload();
    });

    // Unmount before PhysEngine resolves
    unmount();

    // Now resolve – should not cause state updates (no React warnings)
    await act(async () => {
      resolvePhysEngine(makeFakeModule());
    });

    // State should remain in initial loading state (no update after unmount)
    expect(result.current.loading).toBe(true);
    expect(result.current.module).toBeNull();
  });

  it('wraps all expected bridge functions via cwrap', async () => {
    const fakeModule = makeFakeModule();
    window.PhysEngine = vi.fn().mockResolvedValue(fakeModule);

    const { result } = renderHook(() => useWasmModule());

    await act(async () => {
      appendedScript.onload();
    });

    await waitFor(() => expect(result.current.loading).toBe(false));

    const expectedFunctions = [
      'Task_Create',
      'Task_Destroy',
      'Task_GetPositionX',
      'Task_GetPositionY',
      'Task_GetVelocityX',
      'Task_GetVelocityY',
      'Task_GetMass',
      'Task_SetPosition',
      'Task_SetVelocity',
      'Task_SetMass',
      'Engine_IntegrateClassical',
      'State_Serialize',
      'State_Deserialize',
    ];

    const calledNames = fakeModule.cwrap.mock.calls.map((call) => call[0]);
    expectedFunctions.forEach((fnName) => {
      expect(calledNames).toContain(fnName);
    });
  });
});
