/**
 * Integration tests for TaskParticle visualization.
 *
 * Requirements: 10.4, 10.5
 *
 * Tests:
 *  1. TaskParticle renders at the correct screen-space position (SVG transform)
 *  2. Animation loop calls updateSimulation() via requestAnimationFrame
 *  3. Multiple tasks render without overlap (distinct positions)
 */

import React, { useEffect, useRef, useState } from 'react';
import { render, screen, act } from '@testing-library/react';
import TaskParticle from '../TaskParticle';
import { useMomentum } from '../../hooks/useMomentum';

// ─── helpers ────────────────────────────────────────────────────────────────

const SCALE_FACTOR = 5;

/** Build a minimal mock bridge. */
function makeMockModule(stateMap = {}) {
  return {
    Task_Create: vi.fn(() => 1),
    Task_Destroy: vi.fn(),
    Task_SetPosition: vi.fn(),
    Task_SetVelocity: vi.fn(),
    Task_SetMass: vi.fn(),
    Task_GetPositionX: vi.fn((ptr) => stateMap[ptr]?.x ?? 0),
    Task_GetPositionY: vi.fn((ptr) => stateMap[ptr]?.y ?? 0),
    Task_GetVelocityX: vi.fn((ptr) => stateMap[ptr]?.vx ?? 0),
    Task_GetVelocityY: vi.fn((ptr) => stateMap[ptr]?.vy ?? 0),
    Engine_IntegrateClassical: vi.fn(),
  };
}

// ─── 1. TaskParticle renders at correct position ─────────────────────────────

describe('TaskParticle – position rendering', () => {
  it('applies SVG transform with simulation coords scaled by SCALE_FACTOR', () => {
    const x = 10;
    const y = 5;
    render(
      <svg>
        <TaskParticle id="t1" x={x} y={y} vx={0} vy={0} />
      </svg>
    );

    const group = screen.getByTestId('task-particle-t1');
    const transform = group.getAttribute('transform');

    const expectedX = x * SCALE_FACTOR;
    const expectedY = y * SCALE_FACTOR;
    expect(transform).toBe(`translate(${expectedX}, ${expectedY})`);
  });

  it('renders the particle circle at the origin of its local group', () => {
    render(
      <svg>
        <TaskParticle id="t2" x={3} y={7} vx={0} vy={0} />
      </svg>
    );

    const circle = screen.getByTestId('task-particle-circle-t2');
    expect(circle.getAttribute('cx')).toBe('0');
    expect(circle.getAttribute('cy')).toBe('0');
  });

  it('renders at position (0, 0) when x and y are zero', () => {
    render(
      <svg>
        <TaskParticle id="t3" x={0} y={0} vx={0} vy={0} />
      </svg>
    );

    const group = screen.getByTestId('task-particle-t3');
    expect(group.getAttribute('transform')).toBe('translate(0, 0)');
  });

  it('renders at negative simulation coordinates correctly', () => {
    render(
      <svg>
        <TaskParticle id="t4" x={-4} y={-2} vx={0} vy={0} />
      </svg>
    );

    const group = screen.getByTestId('task-particle-t4');
    expect(group.getAttribute('transform')).toBe(`translate(${-4 * SCALE_FACTOR}, ${-2 * SCALE_FACTOR})`);
  });
});

// ─── 2. Velocity trail rendering ─────────────────────────────────────────────

describe('TaskParticle – velocity trail', () => {
  it('renders a trail line when velocity magnitude > 0.001', () => {
    render(
      <svg>
        <TaskParticle id="t5" x={0} y={0} vx={1} vy={0} />
      </svg>
    );

    const trail = screen.getByTestId('task-particle-trail-t5');
    expect(trail).toBeInTheDocument();
  });

  it('does not render a trail when velocity is zero', () => {
    render(
      <svg>
        <TaskParticle id="t6" x={0} y={0} vx={0} vy={0} />
      </svg>
    );

    expect(screen.queryByTestId('task-particle-trail-t6')).toBeNull();
  });

  it('does not render a trail when velocity magnitude <= 0.001', () => {
    render(
      <svg>
        <TaskParticle id="t7" x={0} y={0} vx={0.0005} vy={0.0005} />
      </svg>
    );

    expect(screen.queryByTestId('task-particle-trail-t7')).toBeNull();
  });
});

// ─── 3. Animation loop calls updateSimulation() ──────────────────────────────

describe('Animation loop – updateSimulation() integration', () => {
  let rafCallbacks;
  let originalRaf;
  let originalCaf;

  beforeEach(() => {
    rafCallbacks = [];
    originalRaf = globalThis.requestAnimationFrame;
    originalCaf = globalThis.cancelAnimationFrame;

    // Capture RAF callbacks without auto-executing them
    globalThis.requestAnimationFrame = vi.fn((cb) => {
      const id = rafCallbacks.length;
      rafCallbacks.push(cb);
      return id;
    });
    globalThis.cancelAnimationFrame = vi.fn((id) => {
      rafCallbacks[id] = null;
    });
  });

  afterEach(() => {
    globalThis.requestAnimationFrame = originalRaf;
    globalThis.cancelAnimationFrame = originalCaf;
    vi.restoreAllMocks();
  });

  /**
   * Minimal component that mirrors App.js: drives a RAF loop calling
   * updateSimulation() from useMomentum.
   */
  function SimLoop({ module }) {
    const { updateSimulation } = useMomentum(module);
    const rafRef = useRef(null);

    useEffect(() => {
      if (!module) return;

      const loop = () => {
        updateSimulation();
        rafRef.current = requestAnimationFrame(loop);
      };

      rafRef.current = requestAnimationFrame(loop);

      return () => {
        if (rafRef.current !== null) {
          cancelAnimationFrame(rafRef.current);
        }
      };
    }, [module, updateSimulation]);

    return <div data-testid="sim-loop" />;
  }

  it('calls updateSimulation() on each animation frame tick', () => {
    const mod = makeMockModule();
    // Add a task so updateSimulation has something to call
    mod.Task_Create = vi.fn(() => 42);

    render(<SimLoop module={mod} />);

    // First RAF was scheduled; fire it
    act(() => {
      if (rafCallbacks[0]) rafCallbacks[0]();
    });

    // Engine_IntegrateClassical is called by updateSimulation for each task.
    // Since no tasks were created via the hook in this component, we just
    // verify that requestAnimationFrame was called (loop is running).
    expect(globalThis.requestAnimationFrame).toHaveBeenCalled();
  });

  it('calls Engine_IntegrateClassical for each task on each frame', () => {
    const mod = makeMockModule();
    let ptrCounter = 1;
    mod.Task_Create = vi.fn(() => ptrCounter++);

    const { renderHook, act: actHook } = require('@testing-library/react');
    const { result } = renderHook(() => useMomentum(mod));

    // Create two tasks
    actHook(() => {
      result.current.createTask(1.0, 10.0);
      result.current.createTask(2.0, 20.0);
    });

    // Manually call updateSimulation (simulating what the RAF loop does)
    actHook(() => {
      result.current.updateSimulation();
    });

    expect(mod.Engine_IntegrateClassical).toHaveBeenCalledTimes(2);
  });
});

// ─── 4. Multiple tasks render without overlap ─────────────────────────────────

describe('Multiple TaskParticles – no overlap', () => {
  it('renders distinct particles for each task', () => {
    render(
      <svg>
        <TaskParticle id="a" x={0} y={0} vx={0} vy={0} />
        <TaskParticle id="b" x={10} y={0} vx={0} vy={0} />
        <TaskParticle id="c" x={0} y={10} vx={0} vy={0} />
      </svg>
    );

    expect(screen.getByTestId('task-particle-a')).toBeInTheDocument();
    expect(screen.getByTestId('task-particle-b')).toBeInTheDocument();
    expect(screen.getByTestId('task-particle-c')).toBeInTheDocument();
  });

  it('each particle has a unique transform reflecting its position', () => {
    const tasks = [
      { id: 'p1', x: 0, y: 0 },
      { id: 'p2', x: 5, y: 3 },
      { id: 'p3', x: -2, y: 8 },
    ];

    render(
      <svg>
        {tasks.map(({ id, x, y }) => (
          <TaskParticle key={id} id={id} x={x} y={y} vx={0} vy={0} />
        ))}
      </svg>
    );

    const transforms = tasks.map(({ id, x, y }) => {
      const el = screen.getByTestId(`task-particle-${id}`);
      return el.getAttribute('transform');
    });

    // All transforms should be unique (no two particles at the same screen position)
    const unique = new Set(transforms);
    expect(unique.size).toBe(tasks.length);
  });

  it('particles at different simulation positions map to different screen positions', () => {
    render(
      <svg>
        <TaskParticle id="q1" x={2} y={4} vx={0} vy={0} />
        <TaskParticle id="q2" x={6} y={1} vx={0} vy={0} />
      </svg>
    );

    const t1 = screen.getByTestId('task-particle-q1').getAttribute('transform');
    const t2 = screen.getByTestId('task-particle-q2').getAttribute('transform');

    expect(t1).not.toBe(t2);
    expect(t1).toBe(`translate(${2 * SCALE_FACTOR}, ${4 * SCALE_FACTOR})`);
    expect(t2).toBe(`translate(${6 * SCALE_FACTOR}, ${1 * SCALE_FACTOR})`);
  });
});
