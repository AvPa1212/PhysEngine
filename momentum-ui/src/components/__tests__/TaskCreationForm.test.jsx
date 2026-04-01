/**
 * Unit tests for TaskCreationForm component.
 *
 * Requirements: 7.4, 7.5, 7.6, 7.7
 */

import React from 'react';
import { render, screen, fireEvent } from '@testing-library/react';
import userEvent from '@testing-library/user-event';
import TaskCreationForm from '../TaskCreationForm';

// ─── helpers ────────────────────────────────────────────────────────────────

function renderForm(onCreateTask = vi.fn()) {
  render(<TaskCreationForm onCreateTask={onCreateTask} />);
  return {
    massInput: screen.getByTestId('input-mass'),
    deadlineInput: screen.getByTestId('input-deadline'),
    urgencyInput: screen.getByTestId('input-urgency'),
    kfInput: screen.getByTestId('input-kineticFriction'),
    sfInput: screen.getByTestId('input-staticFriction'),
    submitBtn: screen.getByTestId('submit-button'),
    onCreateTask,
  };
}

// ─── 1. Default values ───────────────────────────────────────────────────────

describe('TaskCreationForm – default values', () => {
  it('renders mass input with default value 1.0', () => {
    const { massInput } = renderForm();
    expect(massInput.value).toBe('1.0');
  });

  it('renders deadline input with default value 10.0', () => {
    const { deadlineInput } = renderForm();
    expect(deadlineInput.value).toBe('10.0');
  });

  it('renders urgency input with default value 100.0', () => {
    const { urgencyInput } = renderForm();
    expect(urgencyInput.value).toBe('100.0');
  });

  it('renders kineticFriction input with default value 0.3', () => {
    const { kfInput } = renderForm();
    expect(kfInput.value).toBe('0.3');
  });

  it('renders staticFriction input with default value 0.5', () => {
    const { sfInput } = renderForm();
    expect(sfInput.value).toBe('0.5');
  });

  it('calls onCreateTask with default values when submitted without changes', () => {
    const { submitBtn, onCreateTask } = renderForm();
    fireEvent.click(submitBtn);
    expect(onCreateTask).toHaveBeenCalledWith(1.0, 10.0, 100.0, 0.3, 0.5);
  });
});

// ─── 2. Successful submission ────────────────────────────────────────────────

describe('TaskCreationForm – form submission', () => {
  it('calls onCreateTask with correct parameters on valid submit', async () => {
    const onCreateTask = vi.fn();
    render(<TaskCreationForm onCreateTask={onCreateTask} />);

    await userEvent.clear(screen.getByTestId('input-mass'));
    await userEvent.type(screen.getByTestId('input-mass'), '2.5');
    await userEvent.clear(screen.getByTestId('input-deadline'));
    await userEvent.type(screen.getByTestId('input-deadline'), '15.0');
    await userEvent.clear(screen.getByTestId('input-urgency'));
    await userEvent.type(screen.getByTestId('input-urgency'), '200.0');
    await userEvent.clear(screen.getByTestId('input-kineticFriction'));
    await userEvent.type(screen.getByTestId('input-kineticFriction'), '0.4');
    await userEvent.clear(screen.getByTestId('input-staticFriction'));
    await userEvent.type(screen.getByTestId('input-staticFriction'), '0.6');

    fireEvent.click(screen.getByTestId('submit-button'));

    expect(onCreateTask).toHaveBeenCalledWith(2.5, 15.0, 200.0, 0.4, 0.6);
  });

  it('resets fields to defaults after successful submission', () => {
    const { massInput, submitBtn } = renderForm();
    fireEvent.change(massInput, { target: { value: '5.0' } });
    fireEvent.click(submitBtn);
    expect(massInput.value).toBe('1.0');
  });

  it('does not call onCreateTask when validation fails', () => {
    const { massInput, submitBtn, onCreateTask } = renderForm();
    fireEvent.change(massInput, { target: { value: '-1' } });
    fireEvent.click(submitBtn);
    expect(onCreateTask).not.toHaveBeenCalled();
  });
});

// ─── 3. Validation – mass ────────────────────────────────────────────────────

describe('TaskCreationForm – mass validation', () => {
  it('shows error when mass is negative', () => {
    const { massInput, submitBtn } = renderForm();
    fireEvent.change(massInput, { target: { value: '-1' } });
    fireEvent.click(submitBtn);
    expect(screen.getByTestId('error-mass')).toBeInTheDocument();
  });

  it('shows error when mass is zero', () => {
    const { massInput, submitBtn } = renderForm();
    fireEvent.change(massInput, { target: { value: '0' } });
    fireEvent.click(submitBtn);
    expect(screen.getByTestId('error-mass')).toBeInTheDocument();
  });

  it('does not show mass error for positive mass', () => {
    const { submitBtn } = renderForm();
    fireEvent.click(submitBtn);
    expect(screen.queryByTestId('error-mass')).toBeNull();
  });
});

// ─── 4. Validation – deadline ────────────────────────────────────────────────

describe('TaskCreationForm – deadline validation', () => {
  it('shows error when deadline is negative', () => {
    const { deadlineInput, submitBtn } = renderForm();
    fireEvent.change(deadlineInput, { target: { value: '-5' } });
    fireEvent.click(submitBtn);
    expect(screen.getByTestId('error-deadline')).toBeInTheDocument();
  });

  it('shows error when deadline is zero', () => {
    const { deadlineInput, submitBtn } = renderForm();
    fireEvent.change(deadlineInput, { target: { value: '0' } });
    fireEvent.click(submitBtn);
    expect(screen.getByTestId('error-deadline')).toBeInTheDocument();
  });
});

// ─── 5. Validation – friction ────────────────────────────────────────────────

describe('TaskCreationForm – friction validation', () => {
  it('shows error when kineticFriction is negative', () => {
    const { kfInput, submitBtn } = renderForm();
    fireEvent.change(kfInput, { target: { value: '-0.1' } });
    fireEvent.click(submitBtn);
    expect(screen.getByTestId('error-kineticFriction')).toBeInTheDocument();
  });

  it('shows error when staticFriction is negative', () => {
    const { sfInput, submitBtn } = renderForm();
    fireEvent.change(sfInput, { target: { value: '-0.2' } });
    fireEvent.click(submitBtn);
    expect(screen.getByTestId('error-staticFriction')).toBeInTheDocument();
  });

  it('allows zero kinetic friction', () => {
    const { kfInput, submitBtn, onCreateTask } = renderForm();
    fireEvent.change(kfInput, { target: { value: '0' } });
    fireEvent.click(submitBtn);
    expect(screen.queryByTestId('error-kineticFriction')).toBeNull();
    expect(onCreateTask).toHaveBeenCalled();
  });

  it('allows zero static friction', () => {
    const { sfInput, submitBtn, onCreateTask } = renderForm();
    fireEvent.change(sfInput, { target: { value: '0' } });
    fireEvent.click(submitBtn);
    expect(screen.queryByTestId('error-staticFriction')).toBeNull();
    expect(onCreateTask).toHaveBeenCalled();
  });
});
