/**
 * Unit tests for useWasmModule hook.
 * Requirements: 10.6
 */

import { renderHook, act, waitFor } from '@testing-library/react';
import { useWasmModule } from './useWasmModule';

function makeFakeModule(overrides = {}) {
  return {
    cwrap: vi.fn((name) => vi.fn().mockReturnValue(`result_of_${name}`)),
    ...overrides,
  };
}

let appendedScript: HTMLScriptElement | null = null;
let originalAppendChild: typeof document.body.appendChild;
let originalRemoveChild: typeof document.body.removeChild;
let originalContains: typeof document.body.contains;

beforeEach(() => {
  appendedScript = null;
  originalAppendChild = document.body.appendChild.bind(document.body);
  originalRemoveChild = document.body.removeChild.bind(document.body);
  originalContains = document.body.contains.bind(document.body);

  vi.spyOn(document.body, 'appendChild').mockImplementation((node: Node) => {
    if ((node as Element).tagName === 'SCRIPT') {
      appendedScript = node as HTMLScriptElement;
      return node;
    }
    return originalAppendChild(node);
  });

  vi.spyOn(document.body, 'removeChild').mockImplementation((node: Node) => {
    if (node === appendedScript) {
      appendedScript = null;
      return node;
    }
    return originalRemoveChild(node);
  });

  vi.spyOn(document.body, 'contains').mockImplementation((node: Node) => {
    if (node === appendedScript) return appendedScript !== null;
    return originalContains(node);
  });
});

afterEach(() => {
  vi.restoreAllMocks();
  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  delete (window as any).PhysEngine;
  appendedScript = null;
});

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
    expect(appendedScript?.src).toContain('/MomentumCore.js');
  });

  it('returns module object with wrapped functions on successful load', async () => {
    const fakeModule = makeFakeModule();
    (window as any).PhysEngine = vi.fn().mockResolvedValue(fakeModule);

    const { result } = renderHook(() => useWasmModule());

    await act(async () => {
      appendedScript?.onload?.(new Event('load') as any);
    });

    await waitFor(() => expect(result.current.loading).toBe(false));

    expect(result.current.error).toBeNull();
    expect(result.current.module).not.toBeNull();
  expect(result.current.module?._raw).toBe(fakeModule);

    const mod = result.current.module!;
    expect(typeof mod.Task_Create).toBe('function');
    expect(typeof mod.Task_Destroy).toBe('function');
    expect(typeof mod.Engine_IntegrateClassical).toBe('function');
    expect(typeof mod.State_Serialize).toBe('function');
    expect(typeof mod.State_Deserialize).toBe('function');
  });

  it('sets error state when script fails to load', async () => {
    const { result } = renderHook(() => useWasmModule());

    await act(async () => {
      appendedScript?.onerror?.(new Event('error') as any);
    });

    await waitFor(() => expect(result.current.loading).toBe(false));

    expect(result.current.module).toBeNull();
    expect(result.current.error).toMatch(/MomentumCore\.js/i);
  });

  it('sets error state when PhysEngine() rejects', async () => {
    (window as any).PhysEngine = vi.fn().mockRejectedValue(new Error('WASM init failed'));

    const { result } = renderHook(() => useWasmModule());

    await act(async () => {
      appendedScript?.onload?.(new Event('load') as any);
    });

    await waitFor(() => expect(result.current.loading).toBe(false));

    expect(result.current.module).toBeNull();
    expect(result.current.error).toBe('WASM init failed');
  });

  it('sets a generic error message when PhysEngine throws a non-Error value', async () => {
    (window as any).PhysEngine = vi.fn().mockRejectedValue('string error');

    const { result } = renderHook(() => useWasmModule());

    await act(async () => {
      appendedScript?.onload?.(new Event('load') as any);
    });

    await waitFor(() => expect(result.current.loading).toBe(false));

    expect(result.current.error).toBeTruthy();
    expect(typeof result.current.error).toBe('string');
  });

  it('removes the injected script on unmount', async () => {
    const fakeModule = makeFakeModule();
    (window as any).PhysEngine = vi.fn().mockResolvedValue(fakeModule);

    const { unmount } = renderHook(() => useWasmModule());

    await act(async () => {
      appendedScript?.onload?.(new Event('load') as any);
    });

    unmount();

    expect(document.body.removeChild).toHaveBeenCalled();
  });

  it('does not update state after unmount (cancelled flag)', async () => {
    let resolvePhysEngine: (value: unknown) => void = () => undefined;
    (window as any).PhysEngine = vi.fn(
      () => new Promise((resolve) => { resolvePhysEngine = resolve; })
    );

    const { result, unmount } = renderHook(() => useWasmModule());

    act(() => {
      appendedScript?.onload?.(new Event('load') as any);
    });

    unmount();

    await act(async () => {
      resolvePhysEngine(makeFakeModule());
    });

    expect(result.current.loading).toBe(true);
    expect(result.current.module).toBeNull();
  });

  it('wraps all expected bridge functions via cwrap', async () => {
    const fakeModule = makeFakeModule();
    (window as any).PhysEngine = vi.fn().mockResolvedValue(fakeModule);

    renderHook(() => useWasmModule());

    await act(async () => {
      appendedScript?.onload?.(new Event('load') as any);
    });

    await waitFor(() => expect(appendedScript).not.toBeNull());

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

    const calledNames = fakeModule.cwrap.mock.calls.map((call: [string]) => call[0]);
    expectedFunctions.forEach((fnName) => {
      expect(calledNames).toContain(fnName);
    });
  });
});