import React, { useState } from 'react';

/**
 * TaskCreationForm – Form for creating a new task with physics parameters.
 *
 * Props:
 *   onCreateTask(mass, deadline, urgency, kineticFriction, staticFriction)
 *     Called on valid form submission.
 *
 * Requirements: 7.1, 7.2, 7.3, 7.4, 7.5, 7.6, 7.7
 */

const DEFAULTS = {
  mass: '1.0',
  deadline: '10.0',
  urgency: '100.0',
  kineticFriction: '0.3',
  staticFriction: '0.5',
};

function TaskCreationForm({ onCreateTask }) {
  const [fields, setFields] = useState(DEFAULTS);
  const [errors, setErrors] = useState({});

  const handleChange = (e) => {
    const { name, value } = e.target;
    setFields((prev) => ({ ...prev, [name]: value }));
    // Clear error for this field on change
    setErrors((prev) => ({ ...prev, [name]: undefined }));
  };

  const validate = () => {
    const errs = {};
    const mass = parseFloat(fields.mass);
    const deadline = parseFloat(fields.deadline);
    const urgency = parseFloat(fields.urgency);
    const kineticFriction = parseFloat(fields.kineticFriction);
    const staticFriction = parseFloat(fields.staticFriction);

    if (isNaN(mass) || mass <= 0) {
      errs.mass = 'Mass must be greater than 0';
    }
    if (isNaN(deadline) || deadline <= 0) {
      errs.deadline = 'Deadline must be greater than 0';
    }
    if (isNaN(urgency)) {
      errs.urgency = 'Urgency must be a number';
    }
    if (isNaN(kineticFriction) || kineticFriction < 0) {
      errs.kineticFriction = 'Kinetic friction must be >= 0';
    }
    if (isNaN(staticFriction) || staticFriction < 0) {
      errs.staticFriction = 'Static friction must be >= 0';
    }

    return errs;
  };

  const handleSubmit = (e) => {
    e.preventDefault();
    const errs = validate();
    if (Object.keys(errs).length > 0) {
      setErrors(errs);
      return;
    }

    const mass = parseFloat(fields.mass);
    const deadline = parseFloat(fields.deadline);
    const urgency = parseFloat(fields.urgency);
    const kineticFriction = parseFloat(fields.kineticFriction);
    const staticFriction = parseFloat(fields.staticFriction);

    onCreateTask(mass, deadline, urgency, kineticFriction, staticFriction);
    setFields(DEFAULTS);
    setErrors({});
  };

  return (
    <form data-testid="task-creation-form" onSubmit={handleSubmit}>
      <div>
        <label htmlFor="mass">Mass (difficulty)</label>
        <input
          id="mass"
          name="mass"
          type="number"
          step="any"
          value={fields.mass}
          onChange={handleChange}
          data-testid="input-mass"
        />
        {errors.mass && (
          <span data-testid="error-mass" role="alert">{errors.mass}</span>
        )}
      </div>

      <div>
        <label htmlFor="deadline">Deadline</label>
        <input
          id="deadline"
          name="deadline"
          type="number"
          step="any"
          value={fields.deadline}
          onChange={handleChange}
          data-testid="input-deadline"
        />
        {errors.deadline && (
          <span data-testid="error-deadline" role="alert">{errors.deadline}</span>
        )}
      </div>

      <div>
        <label htmlFor="urgency">Urgency</label>
        <input
          id="urgency"
          name="urgency"
          type="number"
          step="any"
          value={fields.urgency}
          onChange={handleChange}
          data-testid="input-urgency"
        />
        {errors.urgency && (
          <span data-testid="error-urgency" role="alert">{errors.urgency}</span>
        )}
      </div>

      <div>
        <label htmlFor="kineticFriction">Kinetic Friction</label>
        <input
          id="kineticFriction"
          name="kineticFriction"
          type="number"
          step="any"
          value={fields.kineticFriction}
          onChange={handleChange}
          data-testid="input-kineticFriction"
        />
        {errors.kineticFriction && (
          <span data-testid="error-kineticFriction" role="alert">{errors.kineticFriction}</span>
        )}
      </div>

      <div>
        <label htmlFor="staticFriction">Static Friction</label>
        <input
          id="staticFriction"
          name="staticFriction"
          type="number"
          step="any"
          value={fields.staticFriction}
          onChange={handleChange}
          data-testid="input-staticFriction"
        />
        {errors.staticFriction && (
          <span data-testid="error-staticFriction" role="alert">{errors.staticFriction}</span>
        )}
      </div>

      <button type="submit" data-testid="submit-button">
        Create Task
      </button>
    </form>
  );
}

export default TaskCreationForm;
