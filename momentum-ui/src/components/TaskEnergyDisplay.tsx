import React from 'react';
import EnergyBar from './EnergyBar';
import FocusModeButton from './FocusModeButton';

type TaskEnergyDisplayProps = {
  taskId: string;
  taskPtr?: number;
  kineticEnergy: number;
  potentialEnergy: number;
  totalEnergy: number;
  /** Optional max energy for bar scaling */
  maxEnergy?: number;
  /** Called when the user clicks Focus to inject energy */
  onInject?: (taskPtr: number, amount: number) => void;
};

/**
 * TaskEnergyDisplay – shows energy breakdown for a single task.
 * Renders an EnergyBar with KE/PE proportions and a total energy label.
 * Requirements: 12.2, 12.3, 12.4
 */
function TaskEnergyDisplay({ taskId, taskPtr, kineticEnergy, potentialEnergy, totalEnergy, maxEnergy, onInject }: TaskEnergyDisplayProps) {
  return (
    <div data-testid={`task-energy-display-${taskId}`} style={{ padding: '4px 0' }}>
      <EnergyBar
        kineticEnergy={kineticEnergy}
        potentialEnergy={potentialEnergy}
        maxEnergy={maxEnergy}
      />
      <div
        data-testid={`task-energy-total-${taskId}`}
        style={{ fontSize: 10, color: 'rgba(255,255,255,0.4)', marginTop: 1 }}
      >
        Total: {totalEnergy.toFixed(2)}
      </div>
      {onInject != null && taskPtr != null && (
        <FocusModeButton taskPtr={taskPtr} onInject={onInject} />
      )}
    </div>
  );
}

export default TaskEnergyDisplay;
