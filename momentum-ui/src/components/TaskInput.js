import React, { useState } from 'react';

/**
 * TaskInput – form for injecting a new objective into the workspace.
 * Exposes a title field and a difficulty/mass slider (1–10).
 */
const TaskInput = ({ onAdd }) => {
  const [title, setTitle] = useState('');
  const [difficulty, setDifficulty] = useState(5);

  const handleSubmit = (e) => {
    e.preventDefault();
    const trimmed = title.trim();
    if (!trimmed) return;
    onAdd({ title: trimmed, difficulty: Number(difficulty) });
    setTitle('');
    setDifficulty(5);
  };

  return (
    <form className="input-area" onSubmit={handleSubmit}>
      <input
        type="text"
        value={title}
        onChange={(e) => setTitle(e.target.value)}
        placeholder="Inject new objective..."
        aria-label="Task title"
      />
      <div className="slider-group">
        <label htmlFor="difficulty-slider">
          MASS&nbsp;<span className="neon-text">{difficulty}</span>
        </label>
        <input
          id="difficulty-slider"
          type="range"
          min="1"
          max="10"
          step="1"
          value={difficulty}
          onChange={(e) => setDifficulty(e.target.value)}
          className="difficulty-slider"
          aria-label="Task difficulty / mass"
        />
      </div>
      <button type="submit">Initialize Task</button>
    </form>
  );
};

export default TaskInput;
