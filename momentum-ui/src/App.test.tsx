import { render, screen } from '@testing-library/react';
import { vi } from 'vitest';
import App from './App';

const physicsWorkerState = vi.hoisted(() => ({
  current: {
    isReady: false,
    error: null,
    taskStates: {},
    createTask: vi.fn(),
    destroyTask: vi.fn(),
    applyForce: vi.fn(),
    setMass: vi.fn(),
    collapse: vi.fn(),
    serialize: vi.fn(),
    deserialize: vi.fn(),
    eventBridge: {
      subscribe: vi.fn(() => () => {}),
      clear: vi.fn(),
    },
    perfMonitor: {
      record: vi.fn(),
    },
  },
}));

vi.mock('./hooks/usePhysicsWorker', () => ({
  usePhysicsWorker: () => physicsWorkerState.current,
}));

test('shows the quantum core loading screen while the WASM engine initialises', () => {
  physicsWorkerState.current = {
    ...physicsWorkerState.current,
    isReady: false,
    error: null,
  };

  render(<App />);
  const loader = screen.getByText(/initializing quantum core/i);
  expect(loader).toBeInTheDocument();
});

test('can transition from loading to the live workspace without crashing', () => {
  physicsWorkerState.current = {
    ...physicsWorkerState.current,
    isReady: false,
    error: null,
    taskStates: {},
  };

  const { rerender } = render(<App />);
  expect(screen.getByText(/initializing quantum core/i)).toBeInTheDocument();

  physicsWorkerState.current = {
    ...physicsWorkerState.current,
    isReady: true,
    error: null,
    taskStates: {},
  };

  rerender(<App />);
  expect(screen.getByText(/momentum/i)).toBeInTheDocument();
  expect(screen.getByRole('tablist')).toBeInTheDocument();
});
