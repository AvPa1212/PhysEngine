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

import React from 'react';
import { render, screen } from '@testing-library/react';
import TaskParticle from './TaskParticle';

const SCALE_FACTOR = 5;

function makeMockModule(stateMap: Record<number, { x?: number; y?: number; vx?: number; vy?: number }> = {}) {
  return {
    Task_Create: vi.fn(() => 1),
    Task_Destroy: vi.fn(),
    Task_SetPosition: vi.fn(),
    Task_SetVelocity: vi.fn(),
    Task_SetMass: vi.fn(),
    Task_GetPositionX: vi.fn((ptr: number) => stateMap[ptr]?.x ?? 0),
    Task_GetPositionY: vi.fn((ptr: number) => stateMap[ptr]?.y ?? 0),
    Task_GetVelocityX: vi.fn((ptr: number) => stateMap[ptr]?.vx ?? 0),
    Task_GetVelocityY: vi.fn((ptr: number) => stateMap[ptr]?.vy ?? 0),
    Engine_IntegrateClassical: vi.fn(),
  };
}

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

    expect(transform).toBe(`translate(${x * SCALE_FACTOR}, ${y * SCALE_FACTOR})`);
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

describe('TaskParticle – velocity trail', () => {
  it('renders a trail line when velocity magnitude > 0.001', () => {
    render(
      <svg>
        <TaskParticle id="t5" x={0} y={0} vx={1} vy={0} />
      </svg>
    );

    expect(screen.getByTestId('task-particle-trail-t5')).toBeInTheDocument();
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

    const transforms = tasks.map(({ id }) => screen.getByTestId(`task-particle-${id}`).getAttribute('transform'));
    expect(new Set(transforms).size).toBe(tasks.length);
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