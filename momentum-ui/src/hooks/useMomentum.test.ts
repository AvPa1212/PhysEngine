/**
 * Unit tests for the current useMomentum hook, which loads the WASM engine.
 */

import { renderHook, act, waitFor } from '@testing-library/react';
import { useMomentum } from './useMomentum';

let appendedScript: HTMLScriptElement | null = null;
let originalAppendChild: typeof document.body.appendChild;
let originalRemoveChild: typeof document.body.removeChild;
let originalContains: typeof document.body.contains;

function makeFakeEngine() {
  return {
    cwrap: vi.fn(),
  };
}

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
});

describe('useMomentum', () => {
  it('starts loading with no engine or error', () => {
    const { result } = renderHook(() => useMomentum());

    expect(result.current.engine).toBeNull();
    expect(result.current.isReady).toBe(false);
    expect(result.current.error).toBeNull();
  });

  it('injects the WASM script on mount', () => {
    renderHook(() => useMomentum());

    expect(appendedScript).not.toBeNull();
    expect(appendedScript?.src).toContain('/MomentumCore.js');
  });

  it('resolves the engine factory and updates ready state', async () => {
    const engine = makeFakeEngine();
    (window as any).PhysEngine = vi.fn().mockResolvedValue(engine);

    const { result } = renderHook(() => useMomentum());

    await act(async () => {
      appendedScript?.onload?.(new Event('load') as any);
    });

    await waitFor(() => expect(result.current.isReady).toBe(true));
    expect(result.current.engine).toBe(engine);
    expect(result.current.error).toBeNull();
  });

  it('surfaces load errors', async () => {
    const { result } = renderHook(() => useMomentum());

    await act(async () => {
      appendedScript?.onerror?.(new Event('error') as any);
    });

    await waitFor(() => expect(result.current.isReady).toBe(false));
    expect(result.current.engine).toBeNull();
    expect(result.current.error).toMatch(/MomentumCore\.js/i);
  });

  it('removes the injected script on unmount', async () => {
    const engine = makeFakeEngine();
    (window as any).PhysEngine = vi.fn().mockResolvedValue(engine);

    const { unmount } = renderHook(() => useMomentum());

    await act(async () => {
      appendedScript?.onload?.(new Event('load') as any);
    });

    unmount();
    expect(document.body.removeChild).toHaveBeenCalled();
  });
});