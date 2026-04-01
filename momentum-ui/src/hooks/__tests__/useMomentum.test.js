/**
 * Unit tests for useMomentum hook.
 *
 * Requirements: 11.1, 11.2, 11.3, 11.4
 *
 * Strategy:
 *  - Build a minimal mock bridge that mirrors the shape returned by useWasmModule.
 *  - Use @testing-library/react renderHook + act for clean hook lifecycle testing.
 *  - Verify that each hook function delegates correctly to the bridge and that
 *    React state is updated as expected.
 */

import { renderHook, act } from '@testing-library/react';
import { useMomentum } from '../useMomentum';

// ─── helpers ────────────────────────────────────────────────────────────────

let nextPtr = 1;

/** Build a minimal mock bridge with all functions used by useMomentum. */
function makeMockModule(overrides = {}) {
  return {
    Task_Create: vi.fn(() => nextPtr++),
    Task_Destroy: vi.fn(),
    Task_SetPosition: vi.fn(),
    Task_SetVelocity: vi.fn(),
    Task_SetMass: vi.fn(),
    Task_GetPositionX: vi.fn(() => 1.0),
    Task_GetPositionY: vi.fn(() => 2.0),
    Task_GetVelocityX: vi.fn(() => 3.0),
    Task_GetVelocityY: vi.fn(() => 4.0),
    Engine_IntegrateClassical: vi.fn(),
    ...overrides,
  };
}

// ─── setup / teardown ───────────────────────────────────────────────────────

beforeEach(() => {
  nextPtr = 1;
});

// ─── tests ──────────────────────────────────────────────────────────────────

describe('useMomentum', () => {
  // ── initial state ──────────────────────────────────────────────────────

  it('starts with an empty task list', () => {
    const { result } = renderHook(() => useMomentum(makeMockModule()));
    expect(result.current.tasks).toEqual([]);
  });

  it('returns null module gracefully (module not yet loaded)', () => {
    const { result } = renderHook(() => useMomentum(null));
    expect(result.current.tasks).toEqual([]);
    expect(result.current.createTask(1, 10)).toBeNull();
  });

  // ── createTask ─────────────────────────────────────────────────────────

  it('createTask() adds the returned pointer to tasks state', () => {
    const mod = makeMockModule();
    const { result } = renderHook(() => useMomentum(mod));

    act(() => {
      result.current.createTask(2.0, 10.0);
    });

    expect(result.current.tasks).toHaveLength(1);
    expect(typeof result.current.tasks[0]).toBe('number');
  });

  it('createTask() calls Task_Create on the bridge', () => {
    const mod = makeMockModule();
    const { result } = renderHook(() => useMomentum(mod));

    act(() => {
      result.current.createTask(2.0, 10.0);
    });

    expect(mod.Task_Create).toHaveBeenCalledTimes(1);
  });

  it('createTask() sets initial position to (0, 0)', () => {
    const mod = makeMockModule();
    const { result } = renderHook(() => useMomentum(mod));

    let ptr;
    act(() => {
      ptr = result.current.createTask(2.0, 10.0);
    });

    expect(mod.Task_SetPosition).toHaveBeenCalledWith(ptr, 0, 0);
  });

  it('createTask() sets initial velocity to (0, 0)', () => {
    const mod = makeMockModule();
    const { result } = renderHook(() => useMomentum(mod));

    let ptr;
    act(() => {
      ptr = result.current.createTask(2.0, 10.0);
    });

    expect(mod.Task_SetVelocity).toHaveBeenCalledWith(ptr, 0, 0);
  });

  it('createTask() sets mass on the bridge', () => {
    const mod = makeMockModule();
    const { result } = renderHook(() => useMomentum(mod));

    let ptr;
    act(() => {
      ptr = result.current.createTask(3.5, 10.0);
    });

    expect(mod.Task_SetMass).toHaveBeenCalledWith(ptr, 3.5);
  });

  it('createTask() returns the pointer from Task_Create', () => {
    const mod = makeMockModule();
    const { result } = renderHook(() => useMomentum(mod));

    let ptr;
    act(() => {
      ptr = result.current.createTask(1.0, 5.0);
    });

    expect(ptr).toBe(1); // nextPtr starts at 1
  });

  it('createTask() accumulates multiple tasks in state', () => {
    const mod = makeMockModule();
    const { result } = renderHook(() => useMomentum(mod));

    act(() => {
      result.current.createTask(1.0, 5.0);
      result.current.createTask(2.0, 10.0);
      result.current.createTask(3.0, 15.0);
    });

    expect(result.current.tasks).toHaveLength(3);
  });

  it('createTask() returns null when module is null', () => {
    const { result } = renderHook(() => useMomentum(null));

    let ptr;
    act(() => {
      ptr = result.current.createTask(1.0, 5.0);
    });

    expect(ptr).toBeNull();
  });

  // ── deleteTask ─────────────────────────────────────────────────────────

  it('deleteTask() removes the pointer from tasks state', () => {
    const mod = makeMockModule();
    const { result } = renderHook(() => useMomentum(mod));

    let ptr;
    act(() => {
      ptr = result.current.createTask(1.0, 5.0);
    });

    expect(result.current.tasks).toHaveLength(1);

    act(() => {
      result.current.deleteTask(ptr);
    });

    expect(result.current.tasks).toHaveLength(0);
  });

  it('deleteTask() calls Task_Destroy on the bridge', () => {
    const mod = makeMockModule();
    const { result } = renderHook(() => useMomentum(mod));

    let ptr;
    act(() => {
      ptr = result.current.createTask(1.0, 5.0);
    });

    act(() => {
      result.current.deleteTask(ptr);
    });

    expect(mod.Task_Destroy).toHaveBeenCalledWith(ptr);
  });

  it('deleteTask() only removes the targeted pointer', () => {
    const mod = makeMockModule();
    const { result } = renderHook(() => useMomentum(mod));

    let ptr1, ptr2;
    act(() => {
      ptr1 = result.current.createTask(1.0, 5.0);
      ptr2 = result.current.createTask(2.0, 10.0);
    });

    act(() => {
      result.current.deleteTask(ptr1);
    });

    expect(result.current.tasks).toHaveLength(1);
    expect(result.current.tasks[0]).toBe(ptr2);
  });

  it('deleteTask() does nothing when module is null', () => {
    // Should not throw
    const { result } = renderHook(() => useMomentum(null));
    expect(() => {
      act(() => {
        result.current.deleteTask(42);
      });
    }).not.toThrow();
  });

  // ── updateSimulation ───────────────────────────────────────────────────

  it('updateSimulation() calls Engine_IntegrateClassical for every task', () => {
    const mod = makeMockModule();
    const { result } = renderHook(() => useMomentum(mod));

    let ptr1, ptr2;
    act(() => {
      ptr1 = result.current.createTask(1.0, 5.0);
      ptr2 = result.current.createTask(2.0, 10.0);
    });

    act(() => {
      result.current.updateSimulation();
    });

    expect(mod.Engine_IntegrateClassical).toHaveBeenCalledWith(ptr1);
    expect(mod.Engine_IntegrateClassical).toHaveBeenCalledWith(ptr2);
    expect(mod.Engine_IntegrateClassical).toHaveBeenCalledTimes(2);
  });

  it('updateSimulation() does nothing when there are no tasks', () => {
    const mod = makeMockModule();
    const { result } = renderHook(() => useMomentum(mod));

    act(() => {
      result.current.updateSimulation();
    });

    expect(mod.Engine_IntegrateClassical).not.toHaveBeenCalled();
  });

  it('updateSimulation() does nothing when module is null', () => {
    const { result } = renderHook(() => useMomentum(null));
    expect(() => {
      act(() => {
        result.current.updateSimulation();
      });
    }).not.toThrow();
  });

  it('updateSimulation() accepts an explicit task list', () => {
    const mod = makeMockModule();
    const { result } = renderHook(() => useMomentum(mod));

    act(() => {
      result.current.updateSimulation([10, 20, 30]);
    });

    expect(mod.Engine_IntegrateClassical).toHaveBeenCalledWith(10);
    expect(mod.Engine_IntegrateClassical).toHaveBeenCalledWith(20);
    expect(mod.Engine_IntegrateClassical).toHaveBeenCalledWith(30);
    expect(mod.Engine_IntegrateClassical).toHaveBeenCalledTimes(3);
  });

  // ── getTaskState ───────────────────────────────────────────────────────

  it('getTaskState() returns { x, y, vx, vy } from bridge getters', () => {
    const mod = makeMockModule({
      Task_GetPositionX: vi.fn(() => 5.5),
      Task_GetPositionY: vi.fn(() => 6.6),
      Task_GetVelocityX: vi.fn(() => 7.7),
      Task_GetVelocityY: vi.fn(() => 8.8),
    });
    const { result } = renderHook(() => useMomentum(mod));

    let state;
    act(() => {
      state = result.current.getTaskState(42);
    });

    expect(state).toEqual({ x: 5.5, y: 6.6, vx: 7.7, vy: 8.8 });
  });

  it('getTaskState() calls each getter with the correct pointer', () => {
    const mod = makeMockModule();
    const { result } = renderHook(() => useMomentum(mod));

    act(() => {
      result.current.getTaskState(99);
    });

    expect(mod.Task_GetPositionX).toHaveBeenCalledWith(99);
    expect(mod.Task_GetPositionY).toHaveBeenCalledWith(99);
    expect(mod.Task_GetVelocityX).toHaveBeenCalledWith(99);
    expect(mod.Task_GetVelocityY).toHaveBeenCalledWith(99);
  });

  it('getTaskState() returns null when module is null', () => {
    const { result } = renderHook(() => useMomentum(null));

    let state;
    act(() => {
      state = result.current.getTaskState(1);
    });

    expect(state).toBeNull();
  });
});
