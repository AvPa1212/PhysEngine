import React, { useState, useCallback } from 'react';

const FOCUS_ENERGY_AMOUNT = 50;
const COOLDOWN_MS = 2000;

type FocusModeButtonProps = {
  taskPtr: number;
  onInject: (taskPtr: number, amount: number) => void;
  disabled?: boolean;
};

/**
 * FocusModeButton – injects a fixed energy boost into a task.
 * Enforces a 2-second cooldown after each click.
 * Requirements: 6.1
 */
function FocusModeButton({ taskPtr, onInject, disabled = false }: FocusModeButtonProps) {
  const [cooldown, setCooldown] = useState(false);

  const handleClick = useCallback(() => {
    if (cooldown || disabled) return;
    onInject(taskPtr, FOCUS_ENERGY_AMOUNT);
    setCooldown(true);
    setTimeout(() => setCooldown(false), COOLDOWN_MS);
  }, [cooldown, disabled, onInject, taskPtr]);

  const isDisabled = disabled || cooldown;

  return (
    <button
      data-testid="focus-mode-button"
      onClick={handleClick}
      disabled={isDisabled}
      style={{
        marginTop: 4,
        padding: '2px 10px',
        fontSize: 11,
        borderRadius: 4,
        border: '1px solid rgba(79,142,247,0.5)',
        background: isDisabled ? 'rgba(255,255,255,0.05)' : 'rgba(79,142,247,0.15)',
        color: isDisabled ? 'rgba(255,255,255,0.3)' : '#4f8ef7',
        cursor: isDisabled ? 'not-allowed' : 'pointer',
      }}
    >
      Focus
    </button>
  );
}

export default FocusModeButton;
