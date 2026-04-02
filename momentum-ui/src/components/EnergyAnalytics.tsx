import React from 'react';

interface EnergyAnalyticsProps {
  meanEnergy: number;
  stdDevEnergy: number;
}

const EnergyAnalytics: React.FC<EnergyAnalyticsProps> = ({ meanEnergy, stdDevEnergy }) => {
  return (
    <div data-testid="energy-analytics" className="panel-card">
      <h2>Energy Analytics</h2>
      <div className="metric-row">
        <div className="metric-card">
          <div className="metric-val" data-testid="energy-analytics-mean">
            {meanEnergy.toFixed(2)}
          </div>
          <div className="metric-lbl">Mean Energy</div>
        </div>
        <div className="metric-card">
          <div className="metric-val" data-testid="energy-analytics-stddev">
            {stdDevEnergy.toFixed(2)}
          </div>
          <div className="metric-lbl">Std Dev Energy</div>
        </div>
      </div>
    </div>
  );
};

export default EnergyAnalytics;
