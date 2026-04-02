import React from 'react';

type DampingControlsProps = {
  isDampingEnabled: boolean;
  dampingCoefficient: number;
  onEnableDamping: (coefficient: number) => void;
  onDisableDamping: () => void;
};

export default function DampingControls({
  isDampingEnabled,
  dampingCoefficient,
  onEnableDamping,
  onDisableDamping,
}: DampingControlsProps) {
  const handleToggle = (e: React.ChangeEvent<HTMLInputElement>) => {
    if (e.target.checked) {
      onEnableDamping(dampingCoefficient);
    } else {
      onDisableDamping();
    }
  };

  const handleSlider = (e: React.ChangeEvent<HTMLInputElement>) => {
    const value = Number(e.target.value);
    if (isDampingEnabled) {
      onEnableDamping(value);
    }
  };

  return (
    <div data-testid="damping-controls">
      <label>
        <input
          type="checkbox"
          data-testid="damping-toggle"
          checked={isDampingEnabled}
          onChange={handleToggle}
        />
        {' '}Damping
      </label>
      <div>
        <input
          type="range"
          data-testid="damping-slider"
          min={0}
          max={1}
          step={0.01}
          value={dampingCoefficient}
          onChange={handleSlider}
          disabled={!isDampingEnabled}
        />
        <span data-testid="damping-value">{dampingCoefficient.toFixed(2)}</span>
      </div>
    </div>
  );
}
