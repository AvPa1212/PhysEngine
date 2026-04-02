import React from 'react';

type SystemEnergyGaugeProps = {
  systemEnergy: number;
  /** Max energy for scaling the gauge (defaults to 1000) */
  maxEnergy?: number;
};

/**
 * SystemEnergyGauge – progress bar showing total system energy.
 * Requirements: 12.4
 */
function SystemEnergyGauge({ systemEnergy, maxEnergy = 1000 }: SystemEnergyGaugeProps) {
  const pct = Math.min(100, Math.max(0, (systemEnergy / maxEnergy) * 100));

  return (
    <div data-testid="system-energy-gauge" style={{ width: '100%' }}>
      <div style={{ display: 'flex', justifyContent: 'space-between', marginBottom: 4 }}>
        <span style={{ fontSize: 11, color: 'rgba(255,255,255,0.5)' }}>System Energy</span>
        <span data-testid="system-energy-value" style={{ fontSize: 11, color: '#4f8ef7' }}>
          {systemEnergy.toFixed(1)}
        </span>
      </div>
      <div
        style={{
          height: 6,
          borderRadius: 3,
          background: 'rgba(255,255,255,0.08)',
          overflow: 'hidden',
        }}
      >
        <div
          data-testid="system-energy-bar"
          style={{
            width: `${pct}%`,
            height: '100%',
            background: 'linear-gradient(90deg, #4f8ef7, #a855f7)',
            transition: 'width 0.15s ease',
          }}
        />
      </div>
    </div>
  );
}

export default SystemEnergyGauge;
