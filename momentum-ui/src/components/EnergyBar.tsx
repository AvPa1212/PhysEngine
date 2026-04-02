import React from 'react';

type EnergyBarProps = {
  kineticEnergy: number;
  potentialEnergy: number;
  /** Optional max value to scale bar width against (defaults to total energy) */
  maxEnergy?: number;
};

/**
 * EnergyBar – stacked bar showing Kinetic Energy (blue) and Potential Energy (orange).
 * Bar width is proportional to energy magnitude relative to maxEnergy.
 * Requirements: 12.2, 12.3
 */
function EnergyBar({ kineticEnergy, potentialEnergy, maxEnergy }: EnergyBarProps) {
  const ke = Math.max(0, kineticEnergy);
  const pe = Math.max(0, potentialEnergy);
  const total = ke + pe;
  const scale = maxEnergy != null && maxEnergy > 0 ? maxEnergy : total > 0 ? total : 1;

  const kePct = Math.min(100, (ke / scale) * 100);
  const pePct = Math.min(100, (pe / scale) * 100);

  return (
    <div data-testid="energy-bar" style={{ width: '100%' }}>
      <div
        style={{
          display: 'flex',
          height: 8,
          borderRadius: 4,
          overflow: 'hidden',
          background: 'rgba(255,255,255,0.08)',
        }}
      >
        <div
          data-testid="energy-bar-ke"
          style={{
            width: `${kePct}%`,
            background: '#4f8ef7',
            transition: 'width 0.1s ease',
          }}
        />
        <div
          data-testid="energy-bar-pe"
          style={{
            width: `${pePct}%`,
            background: '#f97316',
            transition: 'width 0.1s ease',
          }}
        />
      </div>
      <div
        data-testid="energy-bar-label"
        style={{ fontSize: 10, color: 'rgba(255,255,255,0.5)', marginTop: 2 }}
      >
        E: {total.toFixed(1)} (KE: {ke.toFixed(1)} / PE: {pe.toFixed(1)})
      </div>
    </div>
  );
}

export default EnergyBar;
