import React, { useState } from 'react';
import { useMomentum } from './hooks/useMomentum';
import QuantumTask from './components/QuantumTask';
import './index.css';

function App() {
  const { engine, isReady } = useMomentum();
  const [tasks, setTasks] = useState([]);
  const [input, setInput] = useState("");
  const isBurningOut = tasks.length > 5;

  const addTask = () => {
    if (!input) return;
    setTasks([...tasks, { id: Date.now(), title: input, difficulty: Math.random() * 5 + 1 }]);
    setInput("");
  };

  if (!isReady) return <div className="loader">LOADING QUANTUM CORE...</div>;

  // Thermodynamic Calculation: Total Entropy of the ToDo List
  const systemHeat = (tasks.length * 12.5).toFixed(1);

  return (
    <div className={`app-container ${isBurningOut ? 'burnout-shake' : ''}`}>
      <div className="app-container">
        <header>
          <h1>MOMENTUM <small>WORKSPACE</small></h1>
          <div className="thermo-meter">
            <span>SYSTEM HEAT: {systemHeat}°K</span>
            <div className="bar"><div className="fill" style={{width: `${systemHeat}%`}}></div></div>
          </div>
        </header>

        <div className="input-area">
          <input 
            value={input} 
            onChange={(e) => setInput(e.target.value)} 
            placeholder="Inject new objective..."
          />
          <button onClick={addTask}>Initialize Task</button>
        </div>

        <div className="task-grid">
          {tasks.map(t => (
            <QuantumTask 
              key={t.id} 
              engine={engine} 
              title={t.title} 
              difficulty={t.difficulty}
              onRemove={() => setTasks(tasks.filter(item => item.id !== t.id))}
            />
          ))}
        </div>
      </div>
    </div>
  );
}

export default App;