import React, { useState } from 'react';
import { useMomentum } from './hooks/useMomentum';
import QuantumTask from './components/QuantumTask';
import TaskInput from './components/TaskInput';
import './index.css';

// System Heat threshold (°K) beyond which the UI enters "Burnout" mode.
const BURNOUT_THRESHOLD = 100;

function App() {
  const { engine, isReady, error } = useMomentum();
  const [tasks, setTasks] = useState([]);

  if (error) {
    return (
      <div className="loader error-state">
        ⚠ ENGINE FAULT: {error}
      </div>
    );
  }

  if (!isReady) {
    return <div className="loader">LOADING QUANTUM CORE...</div>;
  }

  // Thermodynamic Calculation: each active task contributes 12.5 °K of system heat.
  const systemHeat = tasks.length * 12.5;
  const heatDisplay = systemHeat.toFixed(1);
  // Cap the fill bar at 100 % for visual clarity.
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
        <h1>MOMENTUM <small>WORKSPACE</small></h1>
        <div className="thermo-meter">
          <span>SYSTEM HEAT: {heatDisplay}°K</span>
          <div className="bar">
            <div className="fill" style={{ width: `${heatPct}%` }}></div>
          </div>
        </div>
      </header>

      <TaskInput onAdd={addTask} />

      <div className="task-grid">
        {tasks.map((t) => (
          <QuantumTask
            key={t.id}
            engine={engine}
            title={t.title}
            difficulty={t.difficulty}
            onRemove={() => removeTask(t.id)}
          />
        ))}
      </div>
    </div>
  );
}

export default App;