import React, { useEffect, useRef, useState } from 'react';
import { useWasmModule } from './hooks/useWasmModule';
import { useMomentum } from './hooks/useMomentum';
import TaskParticle from './components/TaskParticle';

/**
 * App (JS version) – Minimal simulation app that:
 *  1. Loads the WebAssembly module via useWasmModule
 *  2. Manages tasks via useMomentum
 *  3. Drives a requestAnimationFrame loop at ~60 FPS calling updateSimulation()
 *  4. Renders each task as a TaskParticle inside an SVG canvas
 *
 * Requirements: 10.4, 10.5
 */

const CANVAS_WIDTH = 800;
const CANVAS_HEIGHT = 520;
const ORIGIN_X = CANVAS_WIDTH / 2;
const ORIGIN_Y = CANVAS_HEIGHT / 2;

function App() {
  const { module, loading, error } = useWasmModule();
  const { tasks, createTask, deleteTask, updateSimulation, getTaskState } = useMomentum(module);

  // Force re-render each animation frame so particles move smoothly
  const [, setTick] = useState(0);
  const rafRef = useRef(null);

  // Pause state – when true, updateSimulation() calls are skipped
  const [paused, setPaused] = useState(false);
  const pausedRef = useRef(false);

  // Keep ref in sync with state so the rAF closure always reads the latest value
  useEffect(() => {
    pausedRef.current = paused;
  }, [paused]);

  // Start the animation loop once the module is ready
  useEffect(() => {
    if (!module) return;

    const loop = () => {
      if (!pausedRef.current) {
        updateSimulation();
        setTick((t) => t + 1);
      }
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

  const handlePause = () => setPaused(true);
  const handleResume = () => setPaused(false);

  // Single-step: advance simulation by one tick while paused
  const handleStep = () => {
    if (paused) {
      updateSimulation();
      setTick((t) => t + 1);
    }
  };

  // Collect current state for all tasks
  const particles = tasks.map((ptr) => {
    const state = getTaskState(ptr);
    return state ? { id: ptr, ...state } : null;
  }).filter(Boolean);

  if (loading) {
    return <div data-testid="app-loading">LOADING QUANTUM CORE...</div>;
  }

  if (error) {
    return <div data-testid="app-error">⚠ ENGINE FAULT: {error}</div>;
  }

  return (
    <div data-testid="app-root">
      <div style={{ marginBottom: 8 }}>
        <button
          type="button"
          onClick={() => createTask(1.0, 10.0)}
        >
          Add Task
        </button>
        {!paused ? (
          <button
            type="button"
            data-testid="pause-button"
            onClick={handlePause}
            style={{ marginLeft: 8 }}
          >
            Pause
          </button>
        ) : (
          <button
            type="button"
            data-testid="resume-button"
            onClick={handleResume}
            style={{ marginLeft: 8 }}
          >
            Resume
          </button>
        )}
        {paused && (
          <button
            type="button"
            data-testid="step-button"
            onClick={handleStep}
            style={{ marginLeft: 8 }}
          >
            Step
          </button>
        )}
        <span style={{ marginLeft: 12 }}>Tasks: {tasks.length}</span>
        {paused && <span style={{ marginLeft: 8, color: '#f59e0b' }}>[PAUSED]</span>}
      </div>

      {/* SVG canvas for particle visualization */}
      <svg
        data-testid="simulation-canvas"
        viewBox={`0 0 ${CANVAS_WIDTH} ${CANVAS_HEIGHT}`}
        width={CANVAS_WIDTH}
        height={CANVAS_HEIGHT}
        xmlns="http://www.w3.org/2000/svg"
        style={{ display: 'block', background: '#0a0a0f' }}
      >
        {/* Axis lines */}
        <line x1={0} y1={ORIGIN_Y} x2={CANVAS_WIDTH} y2={ORIGIN_Y} stroke="rgba(255,255,255,0.08)" />
        <line x1={ORIGIN_X} y1={0} x2={ORIGIN_X} y2={CANVAS_HEIGHT} stroke="rgba(255,255,255,0.08)" />

        {/* Task particles centred at canvas origin */}
        <g transform={`translate(${ORIGIN_X}, ${ORIGIN_Y})`}>
          {particles.map(({ id, x, y, vx, vy }) => (
            <TaskParticle key={id} id={id} x={x} y={y} vx={vx} vy={vy} />
          ))}
        </g>
      </svg>
    </div>
  );
}

export default App;
