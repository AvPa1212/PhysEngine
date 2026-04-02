/**
 * Integration tests for energy visualization components.
 *
 * Requirements: 12.2, 12.3, 12.4
 *
 * Tests:
 *  1. EnergyBar renders KE and PE segments with correct proportions
 *  2. EnergyBar displays total energy label
 *  3. EnergyBar handles zero energy gracefully
 *  4. SystemEnergyGauge renders with correct value
 *  5. SystemEnergyGauge scales bar width proportionally
 */

import React from 'react';
import { render, screen } from '@testing-library/react';
import EnergyBar from '../EnergyBar';
import SystemEnergyGauge from '../SystemEnergyGauge';

// ── EnergyBar ────────────────────────────────────────────────────────────────

describe('EnergyBar', () => {
  it('renders the energy bar container', () => {
    render(<EnergyBar kineticEnergy={50} potentialEnergy={50} />);
    expect(screen.getByTestId('energy-bar')).toBeInTheDocument();
  });

  it('renders KE segment with non-zero width when kineticEnergy > 0', () => {
    render(<EnergyBar kineticEnergy={60} potentialEnergy={40} />);
    const keBar = screen.getByTestId('energy-bar-ke');
    const width = keBar.style.width;
    expect(parseFloat(width)).toBeGreaterThan(0);
  });

  it('renders PE segment with non-zero width when potentialEnergy > 0', () => {
    render(<EnergyBar kineticEnergy={60} potentialEnergy={40} />);
    const peBar = screen.getByTestId('energy-bar-pe');
    const width = peBar.style.width;
    expect(parseFloat(width)).toBeGreaterThan(0);
  });

  it('KE segment is wider than PE segment when KE > PE', () => {
    render(<EnergyBar kineticEnergy={75} potentialEnergy={25} />);
    const kePct = parseFloat(screen.getByTestId('energy-bar-ke').style.width);
    const pePct = parseFloat(screen.getByTestId('energy-bar-pe').style.width);
    expect(kePct).toBeGreaterThan(pePct);
  });

  it('PE segment is wider than KE segment when PE > KE', () => {
    render(<EnergyBar kineticEnergy={25} potentialEnergy={75} />);
    const kePct = parseFloat(screen.getByTestId('energy-bar-ke').style.width);
    const pePct = parseFloat(screen.getByTestId('energy-bar-pe').style.width);
    expect(pePct).toBeGreaterThan(kePct);
  });

  it('displays total energy in the label', () => {
    render(<EnergyBar kineticEnergy={30} potentialEnergy={20} />);
    const label = screen.getByTestId('energy-bar-label');
    expect(label.textContent).toContain('50.0');
  });

  it('renders with zero energy without crashing', () => {
    render(<EnergyBar kineticEnergy={0} potentialEnergy={0} />);
    expect(screen.getByTestId('energy-bar')).toBeInTheDocument();
    const keBar = screen.getByTestId('energy-bar-ke');
    expect(parseFloat(keBar.style.width)).toBe(0);
  });

  it('clamps negative energy values to zero', () => {
    render(<EnergyBar kineticEnergy={-10} potentialEnergy={-5} />);
    const keBar = screen.getByTestId('energy-bar-ke');
    const peBar = screen.getByTestId('energy-bar-pe');
    expect(parseFloat(keBar.style.width)).toBe(0);
    expect(parseFloat(peBar.style.width)).toBe(0);
  });

  it('respects maxEnergy prop for scaling', () => {
    // With maxEnergy=200, KE=100 should be 50%
    render(<EnergyBar kineticEnergy={100} potentialEnergy={0} maxEnergy={200} />);
    const kePct = parseFloat(screen.getByTestId('energy-bar-ke').style.width);
    expect(kePct).toBeCloseTo(50, 0);
  });

  it('caps bar width at 100% when energy exceeds maxEnergy', () => {
    render(<EnergyBar kineticEnergy={500} potentialEnergy={500} maxEnergy={100} />);
    const kePct = parseFloat(screen.getByTestId('energy-bar-ke').style.width);
    const pePct = parseFloat(screen.getByTestId('energy-bar-pe').style.width);
    expect(kePct).toBeLessThanOrEqual(100);
    expect(pePct).toBeLessThanOrEqual(100);
  });
});

// ── SystemEnergyGauge ────────────────────────────────────────────────────────

describe('SystemEnergyGauge', () => {
  it('renders the gauge container', () => {
    render(<SystemEnergyGauge systemEnergy={500} />);
    expect(screen.getByTestId('system-energy-gauge')).toBeInTheDocument();
  });

  it('displays the numeric energy value', () => {
    render(<SystemEnergyGauge systemEnergy={123.4} />);
    const value = screen.getByTestId('system-energy-value');
    expect(value.textContent).toContain('123.4');
  });

  it('renders the progress bar', () => {
    render(<SystemEnergyGauge systemEnergy={500} />);
    expect(screen.getByTestId('system-energy-bar')).toBeInTheDocument();
  });

  it('bar width is 50% when systemEnergy is half of maxEnergy', () => {
    render(<SystemEnergyGauge systemEnergy={500} maxEnergy={1000} />);
    const bar = screen.getByTestId('system-energy-bar');
    expect(parseFloat(bar.style.width)).toBeCloseTo(50, 0);
  });

  it('bar width is 100% when systemEnergy equals maxEnergy', () => {
    render(<SystemEnergyGauge systemEnergy={1000} maxEnergy={1000} />);
    const bar = screen.getByTestId('system-energy-bar');
    expect(parseFloat(bar.style.width)).toBe(100);
  });

  it('bar width is 0% when systemEnergy is 0', () => {
    render(<SystemEnergyGauge systemEnergy={0} maxEnergy={1000} />);
    const bar = screen.getByTestId('system-energy-bar');
    expect(parseFloat(bar.style.width)).toBe(0);
  });

  it('caps bar width at 100% when systemEnergy exceeds maxEnergy', () => {
    render(<SystemEnergyGauge systemEnergy={2000} maxEnergy={1000} />);
    const bar = screen.getByTestId('system-energy-bar');
    expect(parseFloat(bar.style.width)).toBe(100);
  });

  it('displays 0.0 for zero energy', () => {
    render(<SystemEnergyGauge systemEnergy={0} />);
    const value = screen.getByTestId('system-energy-value');
    expect(value.textContent).toContain('0.0');
  });
});

// ── FocusModeButton ──────────────────────────────────────────────────────────

import { fireEvent, act } from '@testing-library/react';
import FocusModeButton from '../FocusModeButton';

describe('FocusModeButton', () => {
  beforeEach(() => {
    vi.useFakeTimers();
  });

  afterEach(() => {
    vi.useRealTimers();
  });

  it('renders with "Focus" label', () => {
    render(<FocusModeButton taskPtr={1} onInject={vi.fn()} />);
    expect(screen.getByTestId('focus-mode-button')).toHaveTextContent('Focus');
  });

  it('calls onInject with taskPtr and 50 units on click', () => {
    const onInject = vi.fn();
    render(<FocusModeButton taskPtr={42} onInject={onInject} />);
    fireEvent.click(screen.getByTestId('focus-mode-button'));
    expect(onInject).toHaveBeenCalledWith(42, 50);
  });

  it('is disabled during cooldown after click', () => {
    render(<FocusModeButton taskPtr={1} onInject={vi.fn()} />);
    const btn = screen.getByTestId('focus-mode-button');
    fireEvent.click(btn);
    expect(btn).toBeDisabled();
  });

  it('re-enables after cooldown expires', () => {
    render(<FocusModeButton taskPtr={1} onInject={vi.fn()} />);
    const btn = screen.getByTestId('focus-mode-button');
    fireEvent.click(btn);
    expect(btn).toBeDisabled();
    act(() => { vi.advanceTimersByTime(2000); });
    expect(btn).not.toBeDisabled();
  });

  it('does not call onInject again while in cooldown', () => {
    const onInject = vi.fn();
    render(<FocusModeButton taskPtr={1} onInject={onInject} />);
    const btn = screen.getByTestId('focus-mode-button');
    fireEvent.click(btn);
    fireEvent.click(btn);
    expect(onInject).toHaveBeenCalledTimes(1);
  });
});

// ── DampingControls ──────────────────────────────────────────────────────────

import DampingControls from '../DampingControls';

describe('DampingControls', () => {
  it('renders checkbox and slider', () => {
    render(
      <DampingControls
        isDampingEnabled={false}
        dampingCoefficient={0.1}
        onEnableDamping={vi.fn()}
        onDisableDamping={vi.fn()}
      />
    );
    expect(screen.getByTestId('damping-toggle')).toBeInTheDocument();
    expect(screen.getByTestId('damping-slider')).toBeInTheDocument();
  });

  it('checkbox reflects isDampingEnabled prop', () => {
    render(
      <DampingControls
        isDampingEnabled={true}
        dampingCoefficient={0.3}
        onEnableDamping={vi.fn()}
        onDisableDamping={vi.fn()}
      />
    );
    expect(screen.getByTestId('damping-toggle')).toBeChecked();
  });

  it('calls onEnableDamping when checkbox is checked', () => {
    const onEnable = vi.fn();
    render(
      <DampingControls
        isDampingEnabled={false}
        dampingCoefficient={0.2}
        onEnableDamping={onEnable}
        onDisableDamping={vi.fn()}
      />
    );
    fireEvent.click(screen.getByTestId('damping-toggle'));
    expect(onEnable).toHaveBeenCalledWith(0.2);
  });

  it('calls onDisableDamping when checkbox is unchecked', () => {
    const onDisable = vi.fn();
    render(
      <DampingControls
        isDampingEnabled={true}
        dampingCoefficient={0.2}
        onEnableDamping={vi.fn()}
        onDisableDamping={onDisable}
      />
    );
    fireEvent.click(screen.getByTestId('damping-toggle'));
    expect(onDisable).toHaveBeenCalled();
  });

  it('calls onEnableDamping with new coefficient when slider changes while enabled', () => {
    const onEnable = vi.fn();
    render(
      <DampingControls
        isDampingEnabled={true}
        dampingCoefficient={0.5}
        onEnableDamping={onEnable}
        onDisableDamping={vi.fn()}
      />
    );
    fireEvent.change(screen.getByTestId('damping-slider'), { target: { value: '0.75' } });
    expect(onEnable).toHaveBeenCalledWith(0.75);
  });

  it('displays current coefficient value', () => {
    render(
      <DampingControls
        isDampingEnabled={false}
        dampingCoefficient={0.42}
        onEnableDamping={vi.fn()}
        onDisableDamping={vi.fn()}
      />
    );
    expect(screen.getByTestId('damping-value').textContent).toContain('0.42');
  });

  it('slider is disabled when damping is off', () => {
    render(
      <DampingControls
        isDampingEnabled={false}
        dampingCoefficient={0.1}
        onEnableDamping={vi.fn()}
        onDisableDamping={vi.fn()}
      />
    );
    expect(screen.getByTestId('damping-slider')).toBeDisabled();
  });
});

// ── EnergyAnalytics ──────────────────────────────────────────────────────────

import EnergyAnalytics from '../EnergyAnalytics';

describe('EnergyAnalytics', () => {
  it('renders the analytics container', () => {
    render(<EnergyAnalytics meanEnergy={0} stdDevEnergy={0} />);
    expect(screen.getByTestId('energy-analytics')).toBeInTheDocument();
  });

  it('displays mean energy value', () => {
    render(<EnergyAnalytics meanEnergy={42.5} stdDevEnergy={0} />);
    expect(screen.getByTestId('energy-analytics-mean').textContent).toContain('42.50');
  });

  it('displays standard deviation value', () => {
    render(<EnergyAnalytics meanEnergy={0} stdDevEnergy={7.25} />);
    expect(screen.getByTestId('energy-analytics-stddev').textContent).toContain('7.25');
  });

  it('renders with zero values without crashing', () => {
    render(<EnergyAnalytics meanEnergy={0} stdDevEnergy={0} />);
    expect(screen.getByTestId('energy-analytics-mean').textContent).toContain('0.00');
    expect(screen.getByTestId('energy-analytics-stddev').textContent).toContain('0.00');
  });

  it('displays both mean and stddev labels', () => {
    render(<EnergyAnalytics meanEnergy={10} stdDevEnergy={2} />);
    expect(screen.getByText('Mean Energy')).toBeInTheDocument();
    expect(screen.getByText('Std Dev Energy')).toBeInTheDocument();
  });
});
