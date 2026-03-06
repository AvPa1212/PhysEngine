import React, { useState, useEffect } from 'react';
import { usePhysicsWorker } from './hooks/usePhysicsWorker';
import QuantumTask from './components/QuantumTask';
import TaskInput from './components/TaskInput';
import './index.css';

// System Heat threshold (°K) beyond which the UI enters "Burnout" mode.
const BURNOUT_THRESHOLD = 100;
// Number of active tasks that triggers a SystemOverheat event.
const SYSTEM_OVERHEAT_TASK_COUNT = 8;

function App() {
  const {
    isReady,
    error,
    taskStates,
    createTask,
    destroyTask,
    applyForce,
    collapse,
    eventBridge,
  } = usePhysicsWorker();

  const [tasks, setTasks] = useState([]);
  const [notifications, setNotifications] = useState([]);

  // ── Event subscriptions ──────────────────────────────────────────────────
  useEffect(() => {
    if (!eventBridge) return;

    const unsubEntropy = eventBridge.subscribe(
      'EntropyThresholdReached',
      (evt) => {
        setNotifications((prev) => [
          ...prev.slice(-4),
          {
            id: Date.now(),
            message: `\u26A1 Entropy threshold reached (${Number(
              evt.entropy
            ).toFixed(2)})`,
            type: 'warning',
          },
        ]);
      }
    );

    const unsubOverheat = eventBridge.subscribe('SystemOverheat', () => {
      setNotifications((prev) => [
        ...prev.slice(-4),
        {
          id: Date.now(),
          message: '\uD83D\uDD25 System Overheat! Too many active tasks.',
          type: 'critical',
        },
      ]);
    });

    return () => {
      unsubEntropy();
      unsubOverheat();
    };
  }, [eventBridge]);

  // ── System-overheat detection ────────────────────────────────────────────
  useEffect(() => {
    if (tasks.length >= SYSTEM_OVERHEAT_TASK_COUNT && eventBridge) {
      eventBridge.emit('SystemOverheat', { taskCount: tasks.length });
    }
  }, [tasks.length, eventBridge]);

  // ── Auto-dismiss notifications ───────────────────────────────────────────
  useEffect(() => {
    if (notifications.length === 0) return;
    const timer = setTimeout(() => {
      setNotifications((prev) => prev.slice(1));
    }, 3000);
    return () => clearTimeout(timer);
  }, [notifications]);

  // ── Persist task list to localStorage ────────────────────────────────────
  useEffect(() => {
    try {
      localStorage.setItem('momentum_tasks', JSON.stringify(tasks));
    } catch {
      /* localStorage may be unavailable */
    }
  }, [tasks]);

  // ── Restore tasks from localStorage on initial ready ─────────────────────
  useEffect(() => {
    if (!isReady) return;
    try {
      const saved = localStorage.getItem('momentum_tasks');
      if (saved) {
        const parsed = JSON.parse(saved);
        if (Array.isArray(parsed) && parsed.length > 0) {
          setTasks(parsed);
        }
      }
    } catch {
      /* ignore */
    }
  }, [isReady]);

  // ── Loading / error states ───────────────────────────────────────────────
  if (error) {
    return (
      <div className="loader error-state">
        \u26A0 ENGINE FAULT: {error}
      </div>
    );
  }

  if (!isReady) {
    return <div className="loader">LOADING QUANTUM CORE...</div>;
  }

  // ── Thermodynamic Calculation ────────────────────────────────────────────
  const systemHeat = tasks.length * 12.5;
  const heatDisplay = systemHeat.toFixed(1);
  const heatPct = Math.min((systemHeat / BURNOUT_THRESHOLD) * 100, 100);
  const isBurningOut = systemHeat > BURNOUT_THRESHOLD;

  const addTask = ({ title, difficulty }) => {
    setTasks((prev) => [...prev, { id: Date.now(), title, difficulty }]);
  };

  const removeTask = (id) => {
    setTasks((prev) => prev.filter((t) => t.id !== id));
  };

  return (
    <div className={`app-container${isBurningOut ? ' burnout-shake' : ''}`}>
      <header>
        <h1>
          MOMENTUM <small>WORKSPACE</small>
        </h1>
        <div className="thermo-meter">
          <span>SYSTEM HEAT: {heatDisplay}\u00B0K</span>
          <div className="bar">
            <div className="fill" style={{ width: `${heatPct}%` }}></div>
          </div>
        </div>
      </header>

      {notifications.length > 0 && (
        <div className="notifications">
          {notifications.map((n) => (
            <div key={n.id} className={`notification notification-${n.type}`}>
              {n.message}
            </div>
          ))}
        </div>
      )}

      <TaskInput onAdd={addTask} />

      <div className="task-grid">
        {tasks.map((t) => (
          <QuantumTask
            key={t.id}
            taskId={String(t.id)}
            title={t.title}
            difficulty={t.difficulty}
            taskState={taskStates[String(t.id)]}
            onRemove={() => removeTask(t.id)}
            onApplyForce={applyForce}
            onCollapse={collapse}
            onCreateTask={createTask}
            onDestroyTask={destroyTask}
          />
        ))}
      </div>
    </div>
  );
}

export default App;
