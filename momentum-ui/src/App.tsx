import React, { useEffect, useMemo, useRef, useState } from 'react';
import {
  BrowserRouter,
  Navigate,
  NavLink,
  Route,
  Routes,
  useNavigate,
} from 'react-router-dom';
import { usePhysicsWorker } from './hooks/usePhysicsWorker';
import SystemEnergyGauge from './components/SystemEnergyGauge';
import TaskEnergyDisplay from './components/TaskEnergyDisplay';
import DampingControls from './components/DampingControls';
import EnergyAnalytics from './components/EnergyAnalytics';
import './index.css';

const DEFAULT_GROUP_ID = 'grp-default';
const BURNOUT_THRESHOLD = 100;

type PageKey = 'simulation' | 'tasks' | 'groups' | 'analytics';

type Task = {
  id: string;
  title: string;
  difficulty: number;
  groupId: string;
  createdAt: number;
};

type TaskGroup = {
  id: string;
  name: string;
  color: string;
};

type Notification = {
  id: string;
  message: string;
  type: 'warning' | 'critical';
};

type WorkerTaskState = {
  stressX: number;
  stressY: number;
  stressZ: number;
  entropy: number;
  posX: number;
  posY: number;
  collapseProbability: number;
  stepCount: number;
};

const DEFAULT_GROUP: TaskGroup = {
  id: DEFAULT_GROUP_ID,
  name: 'Ungrouped',
  color: '#4f8ef7',
};

const NAV_ITEMS: Array<{ key: PageKey; label: string; path: string }> = [
  { key: 'simulation', label: 'Simulation', path: '/simulation' },
  { key: 'tasks', label: 'Tasks', path: '/tasks' },
  { key: 'groups', label: 'Groups', path: '/groups' },
  { key: 'analytics', label: 'Analytics', path: '/analytics' },
];

function toRange(value: number, min: number, max: number) {
  return Math.min(max, Math.max(min, value));
}

function App() {
  return (
    <BrowserRouter future={{ v7_startTransition: true, v7_relativeSplatPath: true }}>
      <Workspace />
    </BrowserRouter>
  );
}

function Workspace() {
  const navigate = useNavigate();
  const {
    isReady,
    error,
    taskStates,
    createTask,
    destroyTask,
    applyForce,
    setMass,
    collapse,
    eventBridge,
  } = usePhysicsWorker();

  const [tasks, setTasks] = useState<Task[]>([]);
  const [groups, setGroups] = useState<TaskGroup[]>([DEFAULT_GROUP]);
  const [notifications, setNotifications] = useState<Notification[]>([]);
  const [selectedTaskId, setSelectedTaskId] = useState<string | null>(null);
  const [newTaskTitle, setNewTaskTitle] = useState('');
  const [newTaskDifficulty, setNewTaskDifficulty] = useState(5);
  const [newTaskGroupId, setNewTaskGroupId] = useState(DEFAULT_GROUP_ID);
  const [isQuickAddOpen, setIsQuickAddOpen] = useState(false);
  const [isPaused, setIsPaused] = useState(false);
  const [pausedTaskStates, setPausedTaskStates] = useState<Record<string, WorkerTaskState> | null>(null);
  const [newGroupName, setNewGroupName] = useState('');
  const [newGroupColor, setNewGroupColor] = useState('#4f8ef7');
  const [dragTaskId, setDragTaskId] = useState<string | null>(null);
  const [activeModels, setActiveModels] = useState<Record<string, boolean>>({
    CLASSICAL: true,
    ENERGY: true,
    THERMO: true,
    CHAOS: false,
    QUANTUM: false,
  });
  const [isDampingEnabled, setIsDampingEnabled] = useState(false);
  const [dampingCoefficient, setDampingCoefficient] = useState(0.1);

  const activeTaskIdsRef = useRef<Set<string>>(new Set());
  const taskMassRef = useRef<Record<string, number>>({});

  const groupMap = useMemo(
    () => Object.fromEntries(groups.map((g) => [g.id, g])),
    [groups]
  );

  const selectedTask =
    tasks.find((task) => task.id === selectedTaskId) ?? tasks[0] ?? null;

  const visibleTaskStates = isPaused && pausedTaskStates ? pausedTaskStates : taskStates;

  const totalEnergy = useMemo(
    () => tasks.reduce((sum, task) => sum + task.difficulty * 8.6, 0),
    [tasks]
  );
  const avgDifficulty = useMemo(
    () =>
      tasks.length
        ? tasks.reduce((sum, task) => sum + task.difficulty, 0) / tasks.length
        : 0,
    [tasks]
  );
  const totalEntropy = useMemo(
    () => Object.values(visibleTaskStates).reduce((sum, state) => sum + (state.entropy || 0), 0),
    [visibleTaskStates]
  );

  useEffect(() => {
    try {
      const savedGroups = localStorage.getItem('momentum_groups');
      if (savedGroups) {
        const parsed = JSON.parse(savedGroups);
        if (Array.isArray(parsed)) {
          const hydratedGroups = parsed.filter(
            (g) => g && typeof g.id === 'string' && typeof g.name === 'string'
          ) as TaskGroup[];
          const withDefault = hydratedGroups.some((g) => g.id === DEFAULT_GROUP_ID)
            ? hydratedGroups
            : [DEFAULT_GROUP, ...hydratedGroups];
          setGroups(withDefault);
        }
      }

      const savedTasks = localStorage.getItem('momentum_tasks');
      if (savedTasks) {
        const parsed = JSON.parse(savedTasks);
        if (Array.isArray(parsed)) {
          const hydratedTasks = parsed
            .filter((task) => task && task.title)
            .map((task) => ({
              id: String(task.id),
              title: String(task.title),
              difficulty: toRange(Number(task.difficulty) || 1, 1, 10),
              groupId: task.groupId ? String(task.groupId) : DEFAULT_GROUP_ID,
              createdAt: Number(task.createdAt) || Date.now(),
            })) as Task[];
          setTasks(hydratedTasks);
          if (hydratedTasks[0]) {
            setSelectedTaskId(hydratedTasks[0].id);
          }
        }
      }
    } catch {
      // localStorage may be unavailable
    }
  }, []);

  useEffect(() => {
    if (!eventBridge) return;

    const unsubEntropy = eventBridge.subscribe(
      'EntropyThresholdReached',
      (evt: { taskId: string; entropy: number }) => {
        setNotifications((prev) => [
          ...prev.slice(-4),
          {
            id: `entropy_${evt.taskId}_${Date.now()}`,
            message: `\u26A1 Entropy threshold reached (${Number(evt.entropy).toFixed(2)})`,
            type: 'warning',
          },
        ]);
      }
    );

    const unsubOverheat = eventBridge.subscribe('SystemOverheat', () => {
      setNotifications((prev) => [
        ...prev.slice(-4),
        {
          id: `overheat_${Date.now()}`,
          message: '\uD83D\uDD25 System Overheat! Too many active tasks.',
          type: 'critical',
        },
      ]);
    });

    return () => {
      unsubEntropy();
      unsubOverheat();
    };
  }, [eventBridge]);

  useEffect(() => {
    if (notifications.length === 0) return;
    const timer = setTimeout(() => {
      setNotifications((prev) => prev.slice(1));
    }, 3000);
    return () => clearTimeout(timer);
  }, [notifications]);

  useEffect(() => {
    try {
      localStorage.setItem('momentum_tasks', JSON.stringify(tasks));
    } catch {
      // localStorage may be unavailable
    }
  }, [tasks]);

  useEffect(() => {
    try {
      localStorage.setItem('momentum_groups', JSON.stringify(groups));
    } catch {
      // localStorage may be unavailable
    }
  }, [groups]);

  useEffect(() => {
    if (!isReady) return;
    const activeIds = activeTaskIdsRef.current;
    const desiredIds = new Set(tasks.map((task) => task.id));

    tasks.forEach((task) => {
      if (!activeIds.has(task.id)) {
        createTask(task.id, { mass: task.difficulty });
        activeIds.add(task.id);
      }
      if (taskMassRef.current[task.id] !== task.difficulty) {
        setMass(task.id, task.difficulty);
        taskMassRef.current[task.id] = task.difficulty;
      }
    });

    Array.from(activeIds).forEach((taskId) => {
      if (!desiredIds.has(taskId)) {
        destroyTask(taskId);
        activeIds.delete(taskId);
        delete taskMassRef.current[taskId];
      }
    });
  }, [isReady, tasks, createTask, destroyTask, setMass]);

  useEffect(() => {
    const activeIds = activeTaskIdsRef.current;
    return () => {
      Array.from(activeIds).forEach((taskId) => {
        destroyTask(taskId);
      });
      activeIds.clear();
    };
  }, [destroyTask]);

  useEffect(() => {
    if (!selectedTaskId && tasks[0]) {
      setSelectedTaskId(tasks[0].id);
      return;
    }
    if (selectedTaskId && !tasks.some((task) => task.id === selectedTaskId)) {
      setSelectedTaskId(tasks[0]?.id ?? null);
    }
  }, [selectedTaskId, tasks]);

  if (error) {
    return <div className="loader error-state">\u26A0 ENGINE FAULT: {error}</div>;
  }

  if (!isReady) {
    return <div className="loader">LOADING QUANTUM CORE...</div>;
  }

  const systemHeat = tasks.length * 12.5;
  const heatDisplay = systemHeat.toFixed(1);
  const heatPct = Math.min((systemHeat / BURNOUT_THRESHOLD) * 100, 100);
  const isBurningOut = systemHeat > BURNOUT_THRESHOLD;

  const createTaskFromInput = () => {
    const title = newTaskTitle.trim();
    if (!title) return false;

    const id = `${Date.now()}`;
    const groupId = groupMap[newTaskGroupId] ? newTaskGroupId : DEFAULT_GROUP_ID;

    setTasks((prev) => [
      ...prev,
      {
        id,
        title,
        difficulty: toRange(newTaskDifficulty, 1, 10),
        groupId,
        createdAt: Date.now(),
      },
    ]);

    setNewTaskTitle('');
    setNewTaskDifficulty(5);
    setNewTaskGroupId(groupId);
    setSelectedTaskId(id);
    return true;
  };

  const addTask = (e: React.FormEvent<HTMLFormElement>) => {
    e.preventDefault();
    createTaskFromInput();
  };

  const quickAddTask = () => {
    const created = createTaskFromInput();
    if (!created) return;
    setIsQuickAddOpen(false);
    navigate('/simulation');
  };

  const openQuickAdd = () => {
    setIsQuickAddOpen(true);
    navigate('/simulation');
  };

  const togglePause = () => {
    setIsPaused((prev) => {
      if (prev) {
        setPausedTaskStates(null);
        return false;
      }
      setPausedTaskStates(taskStates as Record<string, WorkerTaskState>);
      return true;
    });
  };

  const patchTask = (id: string, patch: Partial<Omit<Task, 'id' | 'createdAt'>>) => {
    setTasks((prev) =>
      prev.map((task) => {
        if (task.id !== id) {
          return task;
        }
        return {
          ...task,
          ...patch,
          difficulty:
            patch.difficulty != null
              ? toRange(Number(patch.difficulty), 1, 10)
              : task.difficulty,
        };
      })
    );
  };

  const removeTask = (id: string) => {
    setTasks((prev) => prev.filter((task) => task.id !== id));
  };

  const addGroup = (e: React.FormEvent<HTMLFormElement>) => {
    e.preventDefault();
    const name = newGroupName.trim();
    if (!name) return;
    const group: TaskGroup = {
      id: `grp-${Date.now()}`,
      name,
      color: newGroupColor,
    };
    setGroups((prev) => [...prev, group]);
    setNewGroupName('');
  };

  const deleteGroup = (groupId: string) => {
    if (groupId === DEFAULT_GROUP_ID) return;
    setGroups((prev) => prev.filter((group) => group.id !== groupId));
    setTasks((prev) =>
      prev.map((task) =>
        task.groupId === groupId ? { ...task, groupId: DEFAULT_GROUP_ID } : task
      )
    );
    if (newTaskGroupId === groupId) {
      setNewTaskGroupId(DEFAULT_GROUP_ID);
    }
  };

  const moveTask = (taskId: string, targetGroupId: string, targetTaskId?: string) => {
    setTasks((prev) => {
      const dragged = prev.find((task) => task.id === taskId);
      if (!dragged) return prev;

      const withoutDragged = prev.filter((task) => task.id !== taskId);
      const movedTask: Task = { ...dragged, groupId: targetGroupId };

      if (!targetTaskId) {
        let insertAt = withoutDragged.length;
        for (let i = withoutDragged.length - 1; i >= 0; i -= 1) {
          if (withoutDragged[i].groupId === targetGroupId) {
            insertAt = i + 1;
            break;
          }
        }
        withoutDragged.splice(insertAt, 0, movedTask);
        return withoutDragged;
      }

      const targetIndex = withoutDragged.findIndex((task) => task.id === targetTaskId);
      if (targetIndex === -1) {
        withoutDragged.push(movedTask);
      } else {
        withoutDragged.splice(targetIndex, 0, movedTask);
      }
      return withoutDragged;
    });
  };

  const groupedTasks = groups.map((group) => ({
    group,
    tasks: tasks.filter((task) => task.groupId === group.id),
  }));

  const entropyScore = tasks.length ? totalEntropy / tasks.length : 0;

  return (
    <div className={`app${isBurningOut ? ' burnout-shake' : ''}`}>
      <div className="topbar">
        <div className="logo">
          <div className="logo-dot"></div>
          MOMENTUM
        </div>
        <div className="topbar-tabs" role="tablist" aria-label="Workspace pages">
          {NAV_ITEMS.map((item) => (
            <NavLink
              key={item.path}
              to={item.path}
              className={({ isActive }) => `tab ${isActive ? 'active' : ''}`}
            >
              {item.label}
            </NavLink>
          ))}
        </div>
        <div className="topbar-right">
          <button className="btn-sm" type="button" onClick={openQuickAdd}>
            Add Task
          </button>
          <button className="btn-sm" type="button" onClick={togglePause}>
            {isPaused ? 'Resume' : 'Pause'}
          </button>
          <div className="sim-badge">
            <div className="sim-dot"></div>
            LIVE
          </div>
          <div className="sim-badge" aria-label="task count">
            Tasks: {tasks.length}
          </div>
          <button
            className="btn-sm"
            type="button"
            onClick={() => localStorage.setItem('momentum_export', JSON.stringify({ tasks, groups }))}
          >
            Export State
          </button>
        </div>
      </div>

      {notifications.length > 0 && (
        <div className="notifications">
          {notifications.map((n) => (
            <div key={n.id} className={`notification notification-${n.type}`}>
              {n.message}
            </div>
          ))}
        </div>
      )}

      <div className="main-shell">
        <Routes>
          <Route path="/" element={<Navigate to="/simulation" replace />} />

          <Route
            path="/simulation"
            element={
              <div className="page-grid">
                <aside className="sidebar-left">
                  <div className="sidebar-section">
                    <div className="sidebar-label">Active Tasks</div>
                    <button
                      type="button"
                      className="add-btn"
                      onClick={() => setIsQuickAddOpen((prev) => !prev)}
                    >
                      + Add Task
                    </button>
                    {isQuickAddOpen && (
                      <div className="quick-add-panel">
                        <input
                          className="mini-input"
                          type="text"
                          value={newTaskTitle}
                          onChange={(e) => setNewTaskTitle(e.target.value)}
                          placeholder="Task title"
                          aria-label="Task title"
                          autoFocus
                        />
                        <div className="inline-row">
                          <label htmlFor="quick-task-group">Group</label>
                          <select
                            id="quick-task-group"
                            className="mini-input"
                            value={newTaskGroupId}
                            onChange={(e) => setNewTaskGroupId(e.target.value)}
                          >
                            {groups.map((group) => (
                              <option key={group.id} value={group.id}>
                                {group.name}
                              </option>
                            ))}
                          </select>
                        </div>
                        <label className="slider-label" htmlFor="quick-task-difficulty">
                          Difficulty: {newTaskDifficulty}
                        </label>
                        <input
                          id="quick-task-difficulty"
                          type="range"
                          min={1}
                          max={10}
                          value={newTaskDifficulty}
                          onChange={(e) => setNewTaskDifficulty(Number(e.target.value))}
                        />
                        <button type="button" className="btn-primary" onClick={quickAddTask}>
                          Create Task
                        </button>
                      </div>
                    )}
                    {tasks.map((task) => {
                      const state = visibleTaskStates[task.id];
                      const energy = (state?.entropy ?? task.difficulty) * 12;
                      const group = groupMap[task.groupId] ?? DEFAULT_GROUP;
                      return (
                        <button
                          type="button"
                          key={task.id}
                          className={`task-item ${selectedTask?.id === task.id ? 'active' : ''}`}
                          onClick={() => setSelectedTaskId(task.id)}
                        >
                          <div className="task-name">{task.title}</div>
                          <div className="task-meta">
                            <span className="task-tag" style={{ color: group.color, borderColor: `${group.color}55` }}>
                              {group.name}
                            </span>
                            <span className="task-tag tag-m">D: {task.difficulty}</span>
                          </div>
                          <div className="energy-bar-wrap">
                            <div
                              className="energy-bar"
                              style={{ width: `${Math.min(100, energy)}%`, background: group.color }}
                            ></div>
                          </div>
                        </button>
                      );
                    })}
                  </div>

                  {selectedTask && (
                    <div className="sidebar-section inline-editor">
                      <div className="sidebar-label">Quick Edit</div>
                      <input
                        className="mini-input"
                        type="text"
                        value={selectedTask.title}
                        onChange={(e) => patchTask(selectedTask.id, { title: e.target.value })}
                      />
                      <select
                        className="mini-input"
                        value={selectedTask.groupId}
                        onChange={(e) => patchTask(selectedTask.id, { groupId: e.target.value })}
                      >
                        {groups.map((group) => (
                          <option key={group.id} value={group.id}>
                            {group.name}
                          </option>
                        ))}
                      </select>
                      <label className="slider-label" htmlFor="sim-inline-difficulty">
                        Difficulty: {selectedTask.difficulty}
                      </label>
                      <input
                        id="sim-inline-difficulty"
                        type="range"
                        min={1}
                        max={10}
                        value={selectedTask.difficulty}
                        onChange={(e) => patchTask(selectedTask.id, { difficulty: Number(e.target.value) })}
                      />
                      <button
                        type="button"
                        className="btn-sm danger"
                        onClick={() => removeTask(selectedTask.id)}
                      >
                        Remove Task
                      </button>
                    </div>
                  )}
                </aside>

                <section className="canvas-area" aria-label="Engine visualization">
                  <div className="grid-lines"></div>
                  <div className="canvas-overlay-label top-left">PHASE SPACE</div>
                  <div className="canvas-overlay-label top-right">Delta t = 0.016s</div>
                  <svg className="canvas-svg" viewBox="0 0 800 520" xmlns="http://www.w3.org/2000/svg">
                    <line x1="50" y1="470" x2="760" y2="470" stroke="rgba(255,255,255,0.08)" />
                    <line x1="50" y1="60" x2="50" y2="470" stroke="rgba(255,255,255,0.08)" />
                    {tasks.map((task, index) => {
                      const state = visibleTaskStates[task.id];
                      const group = groupMap[task.groupId] ?? DEFAULT_GROUP;
                      const fallbackX = 130 + ((index * 97) % 560);
                      const fallbackY = 130 + ((index * 63) % 280);
                      const x = state?.posX != null ? 400 + state.posX * 100 : fallbackX;
                      const y = state?.posY != null ? 260 + state.posY * 100 : fallbackY;
                      const r = 5 + task.difficulty;
                      return (
                        <g key={task.id} onClick={() => setSelectedTaskId(task.id)}>
                          <circle
                            cx={x}
                            cy={y}
                            r={r + 7}
                            fill={group.color}
                            opacity={selectedTask?.id === task.id ? 0.22 : 0.1}
                          />
                          <circle cx={x} cy={y} r={r} fill={group.color} opacity={0.8} />
                          <text x={x} y={y - r - 10} textAnchor="middle" className="canvas-label">
                            {task.title.slice(0, 18)}
                          </text>
                        </g>
                      );
                    })}
                  </svg>

                  {selectedTask && (
                    <div className="canvas-controls">
                      <button type="button" className="ctrl-btn play" onClick={() => collapse(selectedTask.id)}>
                        Collapse
                      </button>
                      <button
                        type="button"
                        className="ctrl-btn"
                        onClick={() => applyForce(selectedTask.id, 1.5, 0.6, 0.2)}
                      >
                        Push +X
                      </button>
                      <button
                        type="button"
                        className="ctrl-btn"
                        onClick={() => applyForce(selectedTask.id, -1.2, -0.3, 0)}
                      >
                        Pull
                      </button>
                    </div>
                  )}
                </section>

                <aside className="sidebar-right">
                  <div className="panel-block">
                    <div className="panel-title">system state</div>
                    <div className="metric-row">
                      <div className="metric-card">
                        <div className="metric-val">{totalEnergy.toFixed(1)}</div>
                        <div className="metric-lbl">Total Energy</div>
                      </div>
                      <div className="metric-card">
                        <div className="metric-val">{entropyScore.toFixed(2)}</div>
                        <div className="metric-lbl">Entropy</div>
                      </div>
                      <div className="metric-card">
                        <div className="metric-val">{tasks.length}</div>
                        <div className="metric-lbl">Tasks</div>
                      </div>
                      <div className="metric-card">
                        <div className="metric-val">{Math.max(0, 100 - heatPct).toFixed(0)}%</div>
                        <div className="metric-lbl">Stability</div>
                      </div>
                    </div>
                    <div style={{ marginTop: 8 }}>
                      <SystemEnergyGauge systemEnergy={totalEnergy} maxEnergy={Math.max(totalEnergy, 100)} />
                    </div>
                  </div>

                  <div className="panel-block">
                    <div className="panel-title">active models</div>
                    <div className="phase-model-row">
                      {Object.keys(activeModels).map((model) => (
                        <button
                          key={model}
                          type="button"
                          className={`model-pill ${activeModels[model] ? 'on' : 'off'}`}
                          onClick={() =>
                            setActiveModels((prev) => ({
                              ...prev,
                              [model]: !prev[model],
                            }))
                          }
                        >
                          {model}
                        </button>
                      ))}
                    </div>
                    <DampingControls
                      isDampingEnabled={isDampingEnabled}
                      dampingCoefficient={dampingCoefficient}
                      onEnableDamping={(coeff) => {
                        setIsDampingEnabled(true);
                        setDampingCoefficient(coeff);
                      }}
                      onDisableDamping={() => setIsDampingEnabled(false)}
                    />
                  </div>

                  {selectedTask && (
                    <div className="panel-block task-detail">
                      <div className="detail-name">{selectedTask.title}</div>
                      <div className="detail-row">
                        <span className="detail-key">GROUP</span>
                        <span className="detail-val">{(groupMap[selectedTask.groupId] ?? DEFAULT_GROUP).name}</span>
                      </div>
                      <div className="detail-row">
                        <span className="detail-key">DIFFICULTY</span>
                        <span className="detail-val">{selectedTask.difficulty}</span>
                      </div>
                      <div className="detail-row">
                        <span className="detail-key">ENTROPY</span>
                        <span className="detail-val">{(visibleTaskStates[selectedTask.id]?.entropy ?? 0).toFixed(2)}</span>
                      </div>
                      <div className="detail-row">
                        <span className="detail-key">HEAT</span>
                        <span className="detail-val">{heatDisplay}K</span>
                      </div>
                      <div style={{ marginTop: 8 }}>
                        <TaskEnergyDisplay
                          taskId={selectedTask.id}
                          kineticEnergy={(visibleTaskStates[selectedTask.id]?.entropy ?? 0) * 6}
                          potentialEnergy={selectedTask.difficulty * 4}
                          totalEnergy={(visibleTaskStates[selectedTask.id]?.entropy ?? 0) * 6 + selectedTask.difficulty * 4}
                        />
                      </div>
                    </div>
                  )}
                </aside>
              </div>
            }
          />

          <Route
            path="/tasks"
            element={
              <div className="page-simple">
                <div className="panel-card">
                  <h2>Create Task</h2>
                  <form className="task-form" onSubmit={addTask}>
                    <input
                      type="text"
                      value={newTaskTitle}
                      onChange={(e) => setNewTaskTitle(e.target.value)}
                      placeholder="Task title"
                      aria-label="Task title"
                    />
                    <div className="inline-row">
                      <label htmlFor="task-group">Group</label>
                      <select
                        id="task-group"
                        value={newTaskGroupId}
                        onChange={(e) => setNewTaskGroupId(e.target.value)}
                      >
                        {groups.map((group) => (
                          <option key={group.id} value={group.id}>
                            {group.name}
                          </option>
                        ))}
                      </select>
                    </div>
                    <div className="inline-row">
                      <label htmlFor="new-task-difficulty">Difficulty: {newTaskDifficulty}</label>
                      <input
                        id="new-task-difficulty"
                        type="range"
                        min={1}
                        max={10}
                        value={newTaskDifficulty}
                        onChange={(e) => setNewTaskDifficulty(Number(e.target.value))}
                      />
                    </div>
                    <button className="btn-primary" type="submit">
                      Add Task
                    </button>
                  </form>
                </div>

                <div className="panel-card">
                  <h2>Task Board (Drag and Drop)</h2>
                  <div className="group-board">
                    {groupedTasks.map(({ group, tasks: grouped }) => (
                      <section
                        key={group.id}
                        className={`group-column ${dragTaskId ? 'drop-ready' : ''}`}
                        onDragOver={(e) => e.preventDefault()}
                        onDrop={(e) => {
                          e.preventDefault();
                          if (!dragTaskId) return;
                          moveTask(dragTaskId, group.id);
                          setDragTaskId(null);
                        }}
                      >
                        <h3 style={{ color: group.color }}>{group.name}</h3>
                        {grouped.length === 0 && <p className="empty">Drop tasks here</p>}
                        {grouped.map((task) => (
                          <div
                            key={task.id}
                            className="draggable-task"
                            draggable
                            onDragStart={(e) => {
                              e.dataTransfer.effectAllowed = 'move';
                              setDragTaskId(task.id);
                            }}
                            onDragEnd={() => setDragTaskId(null)}
                            onDragOver={(e) => e.preventDefault()}
                            onDrop={(e) => {
                              e.preventDefault();
                              if (!dragTaskId) return;
                              moveTask(dragTaskId, group.id, task.id);
                              setDragTaskId(null);
                            }}
                          >
                            <div className="task-list-title">{task.title}</div>
                            <div className="task-list-meta">
                              <span>Difficulty {task.difficulty}</span>
                            </div>
                            <div className="difficulty-controls" role="group" aria-label={`Difficulty for ${task.title}`}>
                              {[1, 2, 3, 4, 5, 6, 7, 8, 9, 10].map((level) => (
                                <button
                                  type="button"
                                  key={level}
                                  className={task.difficulty === level ? 'level active' : 'level'}
                                  onClick={() => patchTask(task.id, { difficulty: level })}
                                  aria-label={`Set ${task.title} difficulty to ${level}`}
                                >
                                  {level}
                                </button>
                              ))}
                            </div>
                            <div className="task-actions">
                              <button
                                type="button"
                                className="btn-sm"
                                onClick={() => setSelectedTaskId(task.id)}
                              >
                                Focus
                              </button>
                              <button
                                type="button"
                                className="btn-sm danger"
                                onClick={() => removeTask(task.id)}
                              >
                                Remove
                              </button>
                            </div>
                          </div>
                        ))}
                      </section>
                    ))}
                  </div>
                </div>
              </div>
            }
          />

          <Route
            path="/groups"
            element={
              <div className="page-simple">
                <div className="panel-card">
                  <h2>Create Task Group</h2>
                  <form className="task-form" onSubmit={addGroup}>
                    <input
                      type="text"
                      value={newGroupName}
                      onChange={(e) => setNewGroupName(e.target.value)}
                      placeholder="Group name"
                      aria-label="Group name"
                    />
                    <div className="inline-row">
                      <label htmlFor="group-color">Color</label>
                      <input
                        id="group-color"
                        type="color"
                        value={newGroupColor}
                        onChange={(e) => setNewGroupColor(e.target.value)}
                      />
                    </div>
                    <button className="btn-primary" type="submit">
                      Add Group
                    </button>
                  </form>
                </div>

                <div className="panel-card">
                  <h2>Groups</h2>
                  <div className="group-list">
                    {groupedTasks.map(({ group, tasks: grouped }) => (
                      <div className="group-item" key={group.id}>
                        <div>
                          <div className="group-title">
                            <span className="swatch" style={{ background: group.color }}></span>
                            {group.name}
                          </div>
                          <p>{grouped.length} task(s)</p>
                        </div>
                        <button
                          type="button"
                          className="btn-sm"
                          onClick={() => deleteGroup(group.id)}
                          disabled={group.id === DEFAULT_GROUP_ID}
                        >
                          Delete
                        </button>
                      </div>
                    ))}
                  </div>
                </div>
              </div>
            }
          />

          <Route
            path="/analytics"
            element={
              <div className="page-simple analytics-grid">
                <div className="panel-card">
                  <h2>System Heat</h2>
                  <p className="big-stat">{heatDisplay} K</p>
                  <div className="bar">
                    <div className="fill" style={{ width: `${heatPct}%` }}></div>
                  </div>
                </div>
                <div className="panel-card">
                  <h2>Average Difficulty</h2>
                  <p className="big-stat">{avgDifficulty.toFixed(2)}</p>
                </div>
                <div className="panel-card">
                  <h2>Group Balance</h2>
                  <ul className="stat-list">
                    {groupedTasks.map(({ group, tasks: grouped }) => (
                      <li key={group.id}>
                        <span>{group.name}</span>
                        <span>{grouped.length}</span>
                      </li>
                    ))}
                  </ul>
                </div>
                <EnergyAnalytics meanEnergy={0} stdDevEnergy={0} />
              </div>
            }
          />

          <Route path="*" element={<Navigate to="/simulation" replace />} />
        </Routes>
      </div>
    </div>
  );
}

export default App;
