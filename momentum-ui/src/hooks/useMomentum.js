import { useState, useCallback } from 'react';

/**
 * useMomentum – React hook for task lifecycle and simulation operations.
 *
 * Accepts the wrapped bridge object returned by useWasmModule and exposes
 * high-level helpers for creating/destroying tasks, advancing the simulation,
 * and querying per-task state.
 *
 * @param {object|null} module - The wrapped bridge from useWasmModule (or null while loading)
 * @returns {{ tasks, createTask, deleteTask, updateSimulation, getTaskState }}
 *
 * Requirements: 10.2, 10.3, 11.1, 11.2, 11.3, 11.4
 */
export function useMomentum(module) {
  // Array of task pointer numbers (integers returned by Task_Create)
  const [tasks, setTasks] = useState([]);

  /**
   * createTask – allocates a new Task via the bridge, sets its initial
   * physics properties, and adds the pointer to the tasks state.
   *
   * @param {number} mass            - Task difficulty (must be > 0)
   * @param {number} deadline        - Time remaining until deadline (simulation units)
   * @param {number} urgency         - Urgency constant (default 100.0)
   * @param {number} kineticFriction - Kinetic friction coefficient (default 0.3)
   * @param {number} staticFriction  - Static friction coefficient (default 0.5)
   * @returns {number|null} The task pointer, or null if module is not ready
   *
   * Requirements: 11.1
   */
  const createTask = useCallback(
    (mass, deadline, urgency = 100.0, kineticFriction = 0.3, staticFriction = 0.5) => {
      if (!module) return null;

      const ptr = module.Task_Create();

      // Set initial position to origin (0, 0) – Requirement 7.1
      module.Task_SetPosition(ptr, 0, 0);
      // Set initial velocity to zero – Requirement 7.2
      module.Task_SetVelocity(ptr, 0, 0);
      // Set mass (difficulty) – Requirement 7.4
      module.Task_SetMass(ptr, mass);

      // Store deadline and urgency via the bridge if setters are available.
      // The bridge exposes Task_SetDeadline / Task_SetUrgency when compiled;
      // fall back gracefully if they are absent (e.g. in tests that only mock
      // the core setters).
      if (typeof module.Task_SetDeadline === 'function') {
        module.Task_SetDeadline(ptr, deadline);
      }
      if (typeof module.Task_SetUrgency === 'function') {
        module.Task_SetUrgency(ptr, urgency);
      }
      if (typeof module.Task_SetKineticFriction === 'function') {
        module.Task_SetKineticFriction(ptr, kineticFriction);
      }
      if (typeof module.Task_SetStaticFriction === 'function') {
        module.Task_SetStaticFriction(ptr, staticFriction);
      }

      setTasks((prev) => [...prev, ptr]);
      return ptr;
    },
    [module]
  );

  /**
   * deleteTask – destroys a Task via the bridge and removes its pointer from
   * the tasks state.
   *
   * @param {number} taskPtr - The pointer returned by createTask
   *
   * Requirements: 11.1
   */
  const deleteTask = useCallback(
    (taskPtr) => {
      if (!module) return;
      module.Task_Destroy(taskPtr);
      setTasks((prev) => prev.filter((ptr) => ptr !== taskPtr));
    },
    [module]
  );

  /**
   * updateSimulation – advances every tracked task by one TIME_STEP by calling
   * Engine_IntegrateClassical for each pointer in the tasks list.
   *
   * Requirements: 11.3
   */
  const updateSimulation = useCallback(
    (taskList) => {
      if (!module) return;
      // Accept an explicit list (useful in tests / animation loops) or fall
      // back to the current state snapshot captured in the closure.
      const list = taskList !== undefined ? taskList : tasks;
      list.forEach((ptr) => {
        module.Engine_IntegrateClassical(ptr);
      });
    },
    // eslint-disable-next-line react-hooks/exhaustive-deps
    [module, tasks]
  );

  /**
   * getTaskState – queries position and velocity for a single task pointer.
   *
   * @param {number} taskPtr - The pointer returned by createTask
   * @returns {{ x: number, y: number, vx: number, vy: number }|null}
   *
   * Requirements: 11.2
   */
  const getTaskState = useCallback(
    (taskPtr) => {
      if (!module) return null;
      return {
        x: module.Task_GetPositionX(taskPtr),
        y: module.Task_GetPositionY(taskPtr),
        vx: module.Task_GetVelocityX(taskPtr),
        vy: module.Task_GetVelocityY(taskPtr),
      };
    },
    [module]
  );

  return { tasks, createTask, deleteTask, updateSimulation, getTaskState };
}
