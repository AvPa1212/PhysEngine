import React, { useEffect, useRef } from 'react';
import { useWasmModule } from '../hooks/useWasmModule';
import { useMomentum } from '../hooks/useMomentum';
import TaskParticle from './TaskParticle';

/**
 * SimulationCanvas – Drives the classical mechanics simulation loop and
 * renders task particles in a 2D SVG canvas.
 *
 * Calls updateSimulation() inside a requestAnimationFrame loop at ~60 FPS,
 * then maps over all tracked tasks to render a TaskParticle for each one.
 *
 * Props:
 *   taskDefs – array of { id, mass, deadline, urgency } objects describing
 *              the tasks to simulate. Changes to this list are reconciled
 *              automatically (tasks added/removed as needed).
 *   width    – SVG viewport width (default 800)
 *   height   – SVG viewport height (default 520)
 *
 * Requirements: 10.4, 10.5
 */
function SimulationCanvas({ taskDefs = [], width = 800, height = 520 }) {
  const { module, loading, error } = useWasmModule();
  const { tasks, createTask, deleteTask, updateSimulation, getTaskState } = useMomentum(module);

  // Track which task IDs are currently registered in the simulation
  const registeredRef = useRef(new Map()); // id → ptr

  // Reconcile taskDefs with the simulation: create/delete as needed
  useEffect(() => {
    if (!module) return;

    const registered = registeredRef.current;
    const desiredIds = new Set(taskDefs.map((t) => t.id));

    // Create new tasks
    taskDefs.forEach(({ id, mass = 1, deadline = 10, urgency = 100 }) => {
      if (!registered.has(id)) {
        const ptr = createTask(mass, deadline, urgency);
        if (ptr !== null) {
          registered.set(id, ptr);
        }
      }
    });

    // Remove tasks that are no longer in taskDefs
    Array.from(registered.keys()).forEach((id) => {
      if (!desiredIds.has(id)) {
        deleteTask(registered.get(id));
        registered.delete(id);
      }
    });
  }, [module, taskDefs, createTask, deleteTask]);

  // requestAnimationFrame loop at ~60 FPS
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
        rafRef.current = null;
      }
    };
  }, [module, updateSimulation]);

  if (loading) {
    return <div data-testid="simulation-canvas-loading">Loading simulation…</div>;
  }

  if (error) {
    return <div data-testid="simulation-canvas-error">Engine error: {error}</div>;
  }

  // Collect current state for all registered tasks
  const registered = registeredRef.current;
  const particles = Array.from(registered.entries()).map(([id, ptr]) => {
    const state = getTaskState(ptr);
    return state ? { id, ...state } : null;
  }).filter(Boolean);

  // Centre of the SVG canvas acts as the simulation origin
  const originX = width / 2;
  const originY = height / 2;

  return (
    <svg
      data-testid="simulation-canvas"
      viewBox={`0 0 ${width} ${height}`}
      width={width}
      height={height}
      xmlns="http://www.w3.org/2000/svg"
      style={{ display: 'block' }}
    >
      {/* Axis lines */}
      <line x1={0} y1={originY} x2={width} y2={originY} stroke="rgba(255,255,255,0.08)" />
      <line x1={originX} y1={0} x2={originX} y2={height} stroke="rgba(255,255,255,0.08)" />

      {/* Task particles, translated so origin is at canvas centre */}
      <g transform={`translate(${originX}, ${originY})`}>
        {particles.map(({ id, x, y, vx, vy }) => (
          <TaskParticle key={id} id={id} x={x} y={y} vx={vx} vy={vy} />
        ))}
      </g>
    </svg>
  );
}

export default SimulationCanvas;
