import React, { useEffect, useMemo, useRef, useState, useCallback } from 'react';
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
  completed: boolean;
};

type TaskGroup = {
  id: string;
  name: string;
  color: string;
};

type Notification = {
  id: string;
  message: string;
  type: 'warning' | 'critical' | 'success';
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

const NAV_ITEMS: Array<{ key: PageKey; label: string; path: string; icon: string }> = [
  { key: 'simulation', label: 'Simulation', path: '/simulation', icon: '⚛' },
  { key: 'tasks', label: 'Tasks', path: '/tasks', icon: '◈' },
  { key: 'groups', label: 'Groups', path: '/groups', icon: '⬡' },
  { key: 'analytics', label: 'Analytics', path: '/analytics', icon: '◉' },
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

// Animated particle trail hook for canvas
function useAnimatedParticles(tasks: Task[], taskStates: Record<string, WorkerTaskState>, groupMap: Record<string, TaskGroup>) {
  const [tick, setTick] = useState(0);
  const rafRef = useRef<number>(0);
  useEffect(() => {
    let frame = 0;
    const animate = () => {
      frame++;
      if (frame % 2 === 0) setTick(t => t + 1);
      rafRef.current = requestAnimationFrame(animate);
    };
    rafRef.current = requestAnimationFrame(animate);
    return () => cancelAnimationFrame(rafRef.current);
  }, []);
  return tick;
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
  const [energyHistory, setEnergyHistory] = useState<number[]>([]);
  const [entropyHistory, setEntropyHistory] = useState<number[]>([]);
  const [collapseHistory, setCollapseHistory] = useState<number[]>([]);
  const [stressHistory, setStressHistory] = useState<number[]>([]);
  const [showCompleted, setShowCompleted] = useState(false);
  const [forceVector, setForceVector] = useState({ x: 2, y: 0.5, z: 0.2 });
  const [massOverride, setMassOverride] = useState(1);

  const activeTaskIdsRef = useRef<Set<string>>(new Set());
  const taskMassRef = useRef<Record<string, number>>({});
  const tick = useAnimatedParticles(tasks, taskStates, {});

  const groupMap = useMemo(
    () => Object.fromEntries(groups.map((g) => [g.id, g])),
    [groups]
  );

  const selectedTask = tasks.find((task) => task.id === selectedTaskId) ?? tasks[0] ?? null;
  const visibleTaskStates = isPaused && pausedTaskStates ? pausedTaskStates : taskStates;
  const selectedTaskState = selectedTask ? visibleTaskStates[selectedTask.id] : null;

  const activeTasks = useMemo(() => tasks.filter(t => !t.completed), [tasks]);
  const completedTasks = useMemo(() => tasks.filter(t => t.completed), [tasks]);

  const totalEnergy = useMemo(
    () => activeTasks.reduce((sum, task) => sum + task.difficulty * 8.6, 0),
    [activeTasks]
  );
  const avgDifficulty = useMemo(
    () => activeTasks.length ? activeTasks.reduce((sum, task) => sum + task.difficulty, 0) / activeTasks.length : 0,
    [activeTasks]
  );
  const totalEntropy = useMemo(
    () => Object.values(visibleTaskStates).reduce((sum, state) => sum + (state.entropy || 0), 0),
    [visibleTaskStates]
  );
  const averageCollapse = useMemo(() => {
    if (activeTasks.length === 0) return 0;
    return activeTasks.reduce((sum, task) => sum + (visibleTaskStates[task.id]?.collapseProbability ?? 0), 0) / activeTasks.length;
  }, [activeTasks, visibleTaskStates]);
  const activeEnergyDensity = useMemo(() => {
    const canvasArea = Math.max(1, activeTasks.length * 18);
    return totalEnergy / canvasArea;
  }, [activeTasks.length, totalEnergy]);
  const stdDevEnergy = useMemo(() => {
    if (activeTasks.length < 2) return 0;
    const energies = activeTasks.map(t => t.difficulty * 8.6);
    const mean = energies.reduce((a, b) => a + b, 0) / energies.length;
    return Math.sqrt(energies.reduce((sum, e) => sum + (e - mean) ** 2, 0) / energies.length);
  }, [activeTasks]);

  // Track energy/entropy history for sparklines
  useEffect(() => {
    setEnergyHistory(prev => [...prev.slice(-59), totalEnergy]);
    setEntropyHistory(prev => [...prev.slice(-59), totalEntropy]);
    setCollapseHistory(prev => [...prev.slice(-59), averageCollapse * 100]);
    setStressHistory(prev => [...prev.slice(-59), activeEnergyDensity]);
  }, [totalEnergy, totalEntropy, averageCollapse, activeEnergyDensity]);

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
              completed: Boolean(task.completed),
            })) as Task[];
          setTasks(hydratedTasks);
          if (hydratedTasks[0]) setSelectedTaskId(hydratedTasks[0].id);
        }
      }
    } catch { /* localStorage may be unavailable */ }
  }, []);

  useEffect(() => {
    if (!eventBridge) return;
    const unsubEntropy = eventBridge.subscribe('EntropyThresholdReached', (evt: { taskId: string; entropy: number }) => {
      setNotifications((prev) => [...prev.slice(-4), {
        id: `entropy_${evt.taskId}_${Date.now()}`,
        message: `⚡ Entropy threshold reached (${Number(evt.entropy).toFixed(2)})`,
        type: 'warning',
      }]);
    });
    const unsubOverheat = eventBridge.subscribe('SystemOverheat', () => {
      setNotifications((prev) => [...prev.slice(-4), {
        id: `overheat_${Date.now()}`,
        message: '🔥 System Overheat! Too many active tasks.',
        type: 'critical',
      }]);
    });
    return () => { unsubEntropy(); unsubOverheat(); };
  }, [eventBridge]);

  useEffect(() => {
    if (notifications.length === 0) return;
    const timer = setTimeout(() => setNotifications((prev) => prev.slice(1)), 3500);
    return () => clearTimeout(timer);
  }, [notifications]);

  useEffect(() => {
    try { localStorage.setItem('momentum_tasks', JSON.stringify(tasks)); } catch { }
  }, [tasks]);

  useEffect(() => {
    try { localStorage.setItem('momentum_groups', JSON.stringify(groups)); } catch { }
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
      Array.from(activeIds).forEach((taskId) => destroyTask(taskId));
      activeIds.clear();
    };
  }, [destroyTask]);

  useEffect(() => {
    if (!selectedTaskId && tasks[0]) { setSelectedTaskId(tasks[0].id); return; }
    if (selectedTaskId && !tasks.some((task) => task.id === selectedTaskId)) {
      setSelectedTaskId(tasks[0]?.id ?? null);
    }
  }, [selectedTaskId, tasks]);

  useEffect(() => {
    if (!selectedTask) return;
    setMassOverride(selectedTask.difficulty);
  }, [selectedTask?.id]);

  if (error) return <div className="loader error-state">⚠ ENGINE FAULT: {error}</div>;
  if (!isReady) return (
    <div className="loader">
      <div className="loader-ring"></div>
      <div className="loader-text">INITIALIZING QUANTUM CORE</div>
    </div>
  );

  const systemHeat = tasks.length * 12.5;
  const heatDisplay = systemHeat.toFixed(1);
  const heatPct = Math.min((systemHeat / BURNOUT_THRESHOLD) * 100, 100);
  const isBurningOut = systemHeat > BURNOUT_THRESHOLD;
  const entropyScore = activeTasks.length ? totalEntropy / activeTasks.length : 0;

  const createTaskFromInput = () => {
    const title = newTaskTitle.trim();
    if (!title) return false;
    const id = `${Date.now()}`;
    const groupId = groupMap[newTaskGroupId] ? newTaskGroupId : DEFAULT_GROUP_ID;
    setTasks((prev) => [...prev, { id, title, difficulty: toRange(newTaskDifficulty, 1, 10), groupId, createdAt: Date.now(), completed: false }]);
    setNewTaskTitle('');
    setNewTaskDifficulty(5);
    setNewTaskGroupId(groupId);
    setSelectedTaskId(id);
    return true;
  };

  const addTask = (e: React.FormEvent<HTMLFormElement>) => { e.preventDefault(); createTaskFromInput(); };

  const quickAddTask = () => {
    const created = createTaskFromInput();
    if (!created) return;
    setIsQuickAddOpen(false);
    navigate('/simulation');
  };

  const focusSelectedTask = useCallback(() => {
    if (!selectedTask) return;
    navigate('/simulation');
    setSelectedTaskId(selectedTask.id);
    applyForce(selectedTask.id, forceVector.x, forceVector.y, forceVector.z);
  }, [applyForce, forceVector.x, forceVector.y, forceVector.z, navigate, selectedTask]);

  const pulseSelectedTask = useCallback(() => {
    if (!selectedTask) return;
    applyForce(selectedTask.id, forceVector.x * 1.8, forceVector.y * 1.4, forceVector.z * 1.2);
    setNotifications((prev) => [...prev.slice(-4), {
      id: `pulse_${selectedTask.id}_${Date.now()}`,
      message: `⚛ Pulse injected into ${selectedTask.title}`,
      type: 'success',
    }]);
  }, [applyForce, forceVector.x, forceVector.y, forceVector.z, selectedTask]);

  const stabilizeSelectedTask = useCallback(() => {
    if (!selectedTask) return;
    const state = selectedTaskState;
    const fx = -(state?.stressX ?? 0) * 0.8;
    const fy = -(state?.stressY ?? 0) * 0.8;
    const fz = -(state?.stressZ ?? 0) * 0.8;
    applyForce(selectedTask.id, fx, fy, fz);
    setMass(selectedTask.id, Math.max(0.5, massOverride));
  }, [applyForce, massOverride, selectedTask, selectedTaskState, setMass]);

  const hardCollapseSelectedTask = useCallback(() => {
    if (!selectedTask) return;
    collapse(selectedTask.id);
    setNotifications((prev) => [...prev.slice(-4), {
      id: `collapse_${selectedTask.id}_${Date.now()}`,
      message: `⚠ Forced collapse on ${selectedTask.title}`,
      type: 'critical',
    }]);
  }, [collapse, selectedTask]);

  const randomizeForceVector = useCallback(() => {
    const nextX = (Math.random() * 4 - 2).toFixed(2);
    const nextY = (Math.random() * 3 - 1.5).toFixed(2);
    const nextZ = (Math.random() * 2 - 1).toFixed(2);
    setForceVector({ x: Number(nextX), y: Number(nextY), z: Number(nextZ) });
  }, []);

  const toggleComplete = (id: string) => {
    setTasks((prev) => prev.map((t) => {
      if (t.id !== id) return t;
      const nowComplete = !t.completed;
      if (nowComplete) {
        setNotifications(n => [...n.slice(-4), { id: `done_${id}_${Date.now()}`, message: `✓ "${t.title}" completed`, type: 'success' }]);
        collapse(id);
      }
      return { ...t, completed: nowComplete };
    }));
  };

  const togglePause = () => {
    setIsPaused((prev) => {
      if (prev) { setPausedTaskStates(null); return false; }
      setPausedTaskStates(taskStates as Record<string, WorkerTaskState>);
      return true;
    });
  };

  const patchTask = (id: string, patch: Partial<Omit<Task, 'id' | 'createdAt'>>) => {
    setTasks((prev) => prev.map((task) => {
      if (task.id !== id) return task;
      return { ...task, ...patch, difficulty: patch.difficulty != null ? toRange(Number(patch.difficulty), 1, 10) : task.difficulty };
    }));
  };

  const removeTask = (id: string) => setTasks((prev) => prev.filter((task) => task.id !== id));

  const addGroup = (e: React.FormEvent<HTMLFormElement>) => {
    e.preventDefault();
    const name = newGroupName.trim();
    if (!name) return;
    setGroups((prev) => [...prev, { id: `grp-${Date.now()}`, name, color: newGroupColor }]);
    setNewGroupName('');
  };

  const deleteGroup = (groupId: string) => {
    if (groupId === DEFAULT_GROUP_ID) return;
    setGroups((prev) => prev.filter((group) => group.id !== groupId));
    setTasks((prev) => prev.map((task) => task.groupId === groupId ? { ...task, groupId: DEFAULT_GROUP_ID } : task));
    if (newTaskGroupId === groupId) setNewTaskGroupId(DEFAULT_GROUP_ID);
  };

  const moveTask = (taskId: string, targetGroupId: string, targetTaskId?: string) => {
    setTasks((prev) => {
      const dragged = prev.find((task) => task.id === taskId);
      if (!dragged) return prev;
      const withoutDragged = prev.filter((task) => task.id !== taskId);
      const movedTask: Task = { ...dragged, groupId: targetGroupId };
      if (!targetTaskId) {
        let insertAt = withoutDragged.length;
        for (let i = withoutDragged.length - 1; i >= 0; i--) {
          if (withoutDragged[i].groupId === targetGroupId) { insertAt = i + 1; break; }
        }
        withoutDragged.splice(insertAt, 0, movedTask);
        return withoutDragged;
      }
      const targetIndex = withoutDragged.findIndex((task) => task.id === targetTaskId);
      if (targetIndex === -1) withoutDragged.push(movedTask);
      else withoutDragged.splice(targetIndex, 0, movedTask);
      return withoutDragged;
    });
  };

  const groupedTasks = groups.map((group) => ({
    group,
    tasks: tasks.filter((task) => task.groupId === group.id),
  }));

  // Sparkline path generator
  const sparklinePath = (data: number[], w: number, h: number) => {
    if (data.length < 2) return '';
    const max = Math.max(...data, 1);
    const min = Math.min(...data, 0);
    const range = max - min || 1;
    return data.map((v, i) => {
      const x = (i / (data.length - 1)) * w;
      const y = h - ((v - min) / range) * h;
      return `${i === 0 ? 'M' : 'L'}${x.toFixed(1)},${y.toFixed(1)}`;
    }).join(' ');
  };

  return (
    <div className={`app${isBurningOut ? ' burnout-shake' : ''}`}>
      {/* TOPBAR */}
      <div className="topbar">
        <div className="logo">
          <div className="logo-dot"></div>
          MOMENTUM
        </div>
        <div className="topbar-tabs" role="tablist">
          {NAV_ITEMS.map((item) => (
            <NavLink key={item.path} to={item.path} className={({ isActive }) => `tab ${isActive ? 'active' : ''}`}>
              <span className="tab-icon">{item.icon}</span>{item.label}
            </NavLink>
          ))}
        </div>
        <div className="topbar-right">
          <button className="btn-sm accent" type="button" onClick={() => { setIsQuickAddOpen(true); navigate('/simulation'); }}>
            + Add Task
          </button>
          <button className="btn-sm" type="button" onClick={togglePause}>
            {isPaused ? '▶ Resume' : '⏸ Pause'}
          </button>
          <div className="sim-badge">
            <div className="sim-dot"></div>
            {isPaused ? 'PAUSED' : 'LIVE'}
          </div>
          <div className="sim-badge blue">⚡ {activeTasks.length} active</div>
          {completedTasks.length > 0 && <div className="sim-badge green">✓ {completedTasks.length} done</div>}
        </div>
      </div>

      {/* NOTIFICATIONS */}
      {notifications.length > 0 && (
        <div className="notifications">
          {notifications.map((n) => (
            <div key={n.id} className={`notification notification-${n.type}`}>{n.message}</div>
          ))}
        </div>
      )}

      <div className="main-shell">
        <Routes>
          <Route path="/" element={<Navigate to="/simulation" replace />} />

          {/* ── SIMULATION PAGE ── */}
          <Route path="/simulation" element={
            <div className="page-grid">
              {/* LEFT SIDEBAR */}
              <aside className="sidebar-left">
                <div className="sidebar-section">
                  <div className="sidebar-label">Active Tasks ({activeTasks.length})</div>
                  <button type="button" className="add-btn" onClick={() => setIsQuickAddOpen((p) => !p)}>
                    + New Task
                  </button>
                  {isQuickAddOpen && (
                    <div className="quick-add-panel">
                      <input className="mini-input" type="text" value={newTaskTitle}
                        onChange={(e) => setNewTaskTitle(e.target.value)}
                        placeholder="Task title" autoFocus
                        onKeyDown={(e) => e.key === 'Enter' && quickAddTask()}
                      />
                      <div className="inline-row">
                        <label>Group</label>
                        <select className="mini-input" value={newTaskGroupId} onChange={(e) => setNewTaskGroupId(e.target.value)}>
                          {groups.map((g) => <option key={g.id} value={g.id}>{g.name}</option>)}
                        </select>
                      </div>
                      <label className="slider-label">Difficulty: <span className="accent-text">{newTaskDifficulty}</span></label>
                      <input type="range" min={1} max={10} value={newTaskDifficulty} onChange={(e) => setNewTaskDifficulty(Number(e.target.value))} />
                      <button type="button" className="btn-primary" onClick={quickAddTask}>Create Task</button>
                    </div>
                  )}
                  {activeTasks.map((task) => {
                    const state = visibleTaskStates[task.id];
                    const energy = (state?.entropy ?? task.difficulty) * 12;
                    const group = groupMap[task.groupId] ?? DEFAULT_GROUP;
                    const isSelected = selectedTask?.id === task.id;
                    return (
                      <div key={task.id} className={`task-item ${isSelected ? 'active' : ''}`}>
                        <button type="button" className="task-check" onClick={() => toggleComplete(task.id)} title="Mark complete" aria-label={`Complete ${task.title}`}>
                          <span className="check-inner"></span>
                        </button>
                        <button type="button" className="task-body" onClick={() => setSelectedTaskId(task.id)}>
                          <div className="task-name">{task.title}</div>
                          <div className="task-meta">
                            <span className="task-tag" style={{ color: group.color, borderColor: `${group.color}55` }}>{group.name}</span>
                            <span className="task-tag tag-m">D:{task.difficulty}</span>
                            {state && <span className="task-tag tag-e">S:{state.entropy.toFixed(1)}</span>}
                          </div>
                          <div className="energy-bar-wrap">
                            <div className="energy-bar" style={{ width: `${Math.min(100, energy)}%`, background: `linear-gradient(90deg, ${group.color}, ${group.color}88)` }}></div>
                          </div>
                        </button>
                      </div>
                    );
                  })}
                  {completedTasks.length > 0 && (
                    <button type="button" className="show-completed-btn" onClick={() => setShowCompleted(p => !p)}>
                      {showCompleted ? '▲' : '▼'} {completedTasks.length} completed
                    </button>
                  )}
                  {showCompleted && completedTasks.map((task) => {
                    const group = groupMap[task.groupId] ?? DEFAULT_GROUP;
                    return (
                      <div key={task.id} className="task-item completed">
                        <button type="button" className="task-check checked" onClick={() => toggleComplete(task.id)} title="Unmark complete">
                          <span className="check-inner">✓</span>
                        </button>
                        <div className="task-body">
                          <div className="task-name completed-name">{task.title}</div>
                          <div className="task-meta">
                            <span className="task-tag" style={{ color: group.color, borderColor: `${group.color}55` }}>{group.name}</span>
                          </div>
                        </div>
                      </div>
                    );
                  })}
                </div>

                {selectedTask && !selectedTask.completed && (
                  <div className="sidebar-section inline-editor">
                    <div className="sidebar-label">Quick Edit</div>
                    <input className="mini-input" type="text" value={selectedTask.title}
                      onChange={(e) => patchTask(selectedTask.id, { title: e.target.value })} />
                    <select className="mini-input" value={selectedTask.groupId}
                      onChange={(e) => patchTask(selectedTask.id, { groupId: e.target.value })}>
                      {groups.map((g) => <option key={g.id} value={g.id}>{g.name}</option>)}
                    </select>
                    <label className="slider-label">Difficulty: <span className="accent-text">{selectedTask.difficulty}</span></label>
                    <input type="range" min={1} max={10} value={selectedTask.difficulty}
                      onChange={(e) => patchTask(selectedTask.id, { difficulty: Number(e.target.value) })} />
                    <div className="btn-row">
                      <button type="button" className="btn-primary small" onClick={() => toggleComplete(selectedTask.id)}>✓ Complete</button>
                      <button type="button" className="btn-sm danger" onClick={() => removeTask(selectedTask.id)}>✕ Remove</button>
                    </div>
                  </div>
                )}
              </aside>

              {/* CANVAS */}
              <section className="canvas-area" aria-label="Engine visualization">
                <div className="grid-lines"></div>
                <div className="canvas-overlay-label top-left">PHASE SPACE</div>
                <div className="canvas-overlay-label top-right">Δt = 0.016s</div>
                <div className="canvas-overlay-label bottom-left">
                  {Object.entries(activeModels).filter(([,v]) => v).map(([k]) => k).join(' · ')}
                </div>
                <div className="canvas-telemetry">
                  <div className="telemetry-pill">
                    <span>Selected</span>
                    <strong>{selectedTask ? selectedTask.title.slice(0, 18) : 'None'}</strong>
                  </div>
                  <div className="telemetry-pill">
                    <span>Collapse</span>
                    <strong>{(averageCollapse * 100).toFixed(1)}%</strong>
                  </div>
                  <div className="telemetry-pill">
                    <span>Energy Density</span>
                    <strong>{activeEnergyDensity.toFixed(2)}</strong>
                  </div>
                  <div className="telemetry-pill">
                    <span>Telemetry</span>
                    <strong>{activeTasks.length} tasks</strong>
                  </div>
                </div>
                <svg className="canvas-svg" viewBox="0 0 900 600" xmlns="http://www.w3.org/2000/svg" preserveAspectRatio="xMidYMid meet">
                  {/* Grid */}
                  <defs>
                    <radialGradient id="glow-blue" cx="50%" cy="50%" r="50%">
                      <stop offset="0%" stopColor="#4f8ef7" stopOpacity="0.4"/>
                      <stop offset="100%" stopColor="#4f8ef7" stopOpacity="0"/>
                    </radialGradient>
                    <radialGradient id="glow-purple" cx="50%" cy="50%" r="50%">
                      <stop offset="0%" stopColor="#a78bfa" stopOpacity="0.4"/>
                      <stop offset="100%" stopColor="#a78bfa" stopOpacity="0"/>
                    </radialGradient>
                    <filter id="blur-glow">
                      <feGaussianBlur stdDeviation="3" result="blur"/>
                      <feMerge><feMergeNode in="blur"/><feMergeNode in="SourceGraphic"/></feMerge>
                    </filter>
                  </defs>
                  {/* Axes */}
                  <line x1="50" y1="550" x2="860" y2="550" stroke="rgba(255,255,255,0.06)" strokeWidth="1"/>
                  <line x1="50" y1="50" x2="50" y2="550" stroke="rgba(255,255,255,0.06)" strokeWidth="1"/>
                  {/* Grid ticks */}
                  {[0,1,2,3,4,5,6,7].map(i => (
                    <line key={i} x1={50 + i*115} y1="50" x2={50 + i*115} y2="550" stroke="rgba(255,255,255,0.03)" strokeWidth="1"/>
                  ))}
                  {[0,1,2,3,4].map(i => (
                    <line key={i} x1="50" y1={50 + i*125} x2="860" y2={50 + i*125} stroke="rgba(255,255,255,0.03)" strokeWidth="1"/>
                  ))}
                  {/* Energy field background glow for active models */}
                  {activeModels.QUANTUM && <ellipse cx="450" cy="300" rx="300" ry="200" fill="url(#glow-purple)" opacity="0.3"/>}
                  {activeModels.CHAOS && <ellipse cx="450" cy="300" rx="350" ry="250" fill="url(#glow-blue)" opacity="0.15"/>}
                  {/* Task particles */}
                  {activeTasks.map((task, index) => {
                    const state = visibleTaskStates[task.id];
                    const group = groupMap[task.groupId] ?? DEFAULT_GROUP;
                    const fallbackX = 130 + ((index * 97) % 700);
                    const fallbackY = 100 + ((index * 63) % 400);
                    const baseX = state?.posX != null ? 450 + state.posX * 120 : fallbackX;
                    const baseY = state?.posY != null ? 300 + state.posY * 120 : fallbackY;
                    // Animate with tick for liveliness
                    const wobble = activeModels.CHAOS ? Math.sin(tick * 0.05 + index) * 4 : 0;
                    const x = Math.max(60, Math.min(840, baseX + wobble));
                    const y = Math.max(60, Math.min(540, baseY + Math.cos(tick * 0.04 + index) * (activeModels.CHAOS ? 3 : 0)));
                    const r = 5 + task.difficulty * 1.2;
                    const isSelected = selectedTask?.id === task.id;
                    const entropy = state?.entropy ?? 0;
                    const collapsePct = state?.collapseProbability ?? 0;
                    // Quantum ring for quantum model
                    const quantumRingR = r + 12 + Math.sin(tick * 0.08 + index) * 3;
                    return (
                      <g key={task.id} onClick={() => setSelectedTaskId(task.id)} style={{ cursor: 'pointer' }} filter={isSelected ? 'url(#blur-glow)' : undefined}>
                        {/* Outer glow */}
                        <circle cx={x} cy={y} r={r + 14} fill={group.color} opacity={isSelected ? 0.18 : 0.07}/>
                        {/* Quantum ring */}
                        {activeModels.QUANTUM && (
                          <circle cx={x} cy={y} r={quantumRingR} fill="none" stroke={group.color} strokeWidth="0.8"
                            strokeDasharray={`${collapsePct * 2} ${(1 - collapsePct) * 2}`} opacity="0.5"/>
                        )}
                        {/* Velocity trail */}
                        {state && (Math.abs(state.stressX) + Math.abs(state.stressY) > 0.1) && (
                          <line x1={x} y1={y} x2={x - state.stressX * 20} y2={y - state.stressY * 20}
                            stroke={group.color} strokeWidth="1.5" strokeLinecap="round" opacity="0.4"/>
                        )}
                        {/* Entropy halo */}
                        {entropy > 1 && <circle cx={x} cy={y} r={r + 6 + entropy * 0.5} fill="none" stroke={group.color} strokeWidth="0.5" opacity="0.3"/>}
                        {/* Main particle */}
                        <circle cx={x} cy={y} r={r} fill={group.color} opacity={0.85}/>
                        {/* Inner highlight */}
                        <circle cx={x - r * 0.3} cy={y - r * 0.3} r={r * 0.35} fill="white" opacity="0.2"/>
                        {/* Label */}
                        <text x={x} y={y - r - 8} textAnchor="middle" className="canvas-label">{task.title.slice(0, 16)}</text>
                        {/* Difficulty badge */}
                        <text x={x} y={y + 4} textAnchor="middle" className="canvas-badge">{task.difficulty}</text>
                      </g>
                    );
                  })}
                  {/* Completed tasks as faded dots */}
                  {completedTasks.map((task, index) => {
                    const group = groupMap[task.groupId] ?? DEFAULT_GROUP;
                    const x = 80 + ((index * 60) % 780);
                    const y = 570;
                    return (
                      <g key={task.id} opacity="0.3">
                        <circle cx={x} cy={y} r={4} fill={group.color}/>
                        <text x={x} y={y - 8} textAnchor="middle" className="canvas-label" fontSize="8">✓</text>
                      </g>
                    );
                  })}
                  {/* Axis labels */}
                  <text x="455" y="595" textAnchor="middle" className="canvas-label" opacity="0.4">Position X</text>
                  <text x="20" y="305" textAnchor="middle" className="canvas-label" opacity="0.4" transform="rotate(-90,20,305)">Position Y</text>
                </svg>

                {selectedTask && !selectedTask.completed && (
                  <div className="canvas-controls">
                    <span className="ctrl-label">{selectedTask.title.slice(0, 12)}</span>
                    <div className="ctrl-divider"/>
                    <button type="button" className="ctrl-btn play" onClick={hardCollapseSelectedTask} title="Collapse quantum state">⊗ Collapse</button>
                    <button type="button" className="ctrl-btn" onClick={focusSelectedTask} title="Inject force vector">→ Focus</button>
                    <button type="button" className="ctrl-btn" onClick={pulseSelectedTask} title="Apply kinetic pulse">⚡ Pulse</button>
                    <button type="button" className="ctrl-btn" onClick={stabilizeSelectedTask} title="Stabilize selected task">◌ Stabilize</button>
                    <button type="button" className="ctrl-btn accent" onClick={() => toggleComplete(selectedTask.id)} title="Mark complete">✓ Done</button>
                  </div>
                )}
              </section>

              {/* RIGHT SIDEBAR */}
              <aside className="sidebar-right">
                <div className="panel-block">
                  <div className="panel-title">system state</div>
                  <div className="metric-row">
                    <div className="metric-card">
                      <div className="metric-val accent">{totalEnergy.toFixed(1)}</div>
                      <div className="metric-lbl">Total Energy</div>
                    </div>
                    <div className="metric-card">
                      <div className="metric-val purple">{entropyScore.toFixed(2)}</div>
                      <div className="metric-lbl">Avg Entropy</div>
                    </div>
                    <div className="metric-card">
                      <div className="metric-val green">{completedTasks.length}</div>
                      <div className="metric-lbl">Completed</div>
                    </div>
                    <div className="metric-card">
                      <div className="metric-val" style={{ color: heatPct > 80 ? 'var(--coral)' : 'var(--amber)' }}>{Math.max(0, 100 - heatPct).toFixed(0)}%</div>
                      <div className="metric-lbl">Stability</div>
                    </div>
                  </div>
                  <SystemEnergyGauge systemEnergy={totalEnergy} maxEnergy={Math.max(totalEnergy, 100)} />
                  {/* Energy sparkline */}
                  <div className="sparkline-wrap">
                    <div className="sparkline-label">Energy History</div>
                    <svg width="100%" height="36" viewBox={`0 0 200 36`} preserveAspectRatio="none">
                      <path d={sparklinePath(energyHistory, 200, 36)} fill="none" stroke="var(--accent)" strokeWidth="1.5" opacity="0.7"/>
                      <path d={sparklinePath(energyHistory, 200, 36) + ' L200,36 L0,36 Z'} fill="var(--accent)" opacity="0.08"/>
                    </svg>
                  </div>
                  <div className="sparkline-wrap">
                    <div className="sparkline-label">Entropy History</div>
                    <svg width="100%" height="36" viewBox={`0 0 200 36`} preserveAspectRatio="none">
                      <path d={sparklinePath(entropyHistory, 200, 36)} fill="none" stroke="var(--accent2)" strokeWidth="1.5" opacity="0.7"/>
                      <path d={sparklinePath(entropyHistory, 200, 36) + ' L200,36 L0,36 Z'} fill="var(--accent2)" opacity="0.08"/>
                    </svg>
                  </div>
                  <div className="sparkline-wrap">
                    <div className="sparkline-label">Collapse Pressure</div>
                    <svg width="100%" height="36" viewBox={`0 0 200 36`} preserveAspectRatio="none">
                      <path d={sparklinePath(collapseHistory, 200, 36)} fill="none" stroke="var(--coral)" strokeWidth="1.5" opacity="0.7"/>
                      <path d={sparklinePath(collapseHistory, 200, 36) + ' L200,36 L0,36 Z'} fill="var(--coral)" opacity="0.08"/>
                    </svg>
                  </div>
                  <div className="sparkline-wrap">
                    <div className="sparkline-label">Stress Density</div>
                    <svg width="100%" height="36" viewBox={`0 0 200 36`} preserveAspectRatio="none">
                      <path d={sparklinePath(stressHistory, 200, 36)} fill="none" stroke="var(--green)" strokeWidth="1.5" opacity="0.7"/>
                      <path d={sparklinePath(stressHistory, 200, 36) + ' L200,36 L0,36 Z'} fill="var(--green)" opacity="0.08"/>
                    </svg>
                  </div>
                </div>

                <div className="panel-block control-surface">
                  <div className="panel-title">task control surface</div>
                  {selectedTask ? (
                    <>
                      <div className="control-surface-head">
                        <div>
                          <div className="surface-label">Selected Task</div>
                          <div className="surface-title">{selectedTask.title}</div>
                        </div>
                        <button type="button" className="btn-sm accent-btn" onClick={randomizeForceVector}>Randomize Vector</button>
                      </div>
                      <div className="control-grid">
                        <label className="control-row">
                          <span>Mass Override <strong>{massOverride.toFixed(1)}</strong></span>
                          <input type="range" min={0.5} max={12} step={0.1} value={massOverride} onChange={(e) => setMassOverride(Number(e.target.value))} />
                        </label>
                        <label className="control-row">
                          <span>Force X <strong>{forceVector.x.toFixed(2)}</strong></span>
                          <input type="range" min={-4} max={4} step={0.05} value={forceVector.x} onChange={(e) => setForceVector((prev) => ({ ...prev, x: Number(e.target.value) }))} />
                        </label>
                        <label className="control-row">
                          <span>Force Y <strong>{forceVector.y.toFixed(2)}</strong></span>
                          <input type="range" min={-4} max={4} step={0.05} value={forceVector.y} onChange={(e) => setForceVector((prev) => ({ ...prev, y: Number(e.target.value) }))} />
                        </label>
                        <label className="control-row">
                          <span>Force Z <strong>{forceVector.z.toFixed(2)}</strong></span>
                          <input type="range" min={-3} max={3} step={0.05} value={forceVector.z} onChange={(e) => setForceVector((prev) => ({ ...prev, z: Number(e.target.value) }))} />
                        </label>
                      </div>
                      <div className="surface-stats">
                        <div className="surface-stat"><span>Entropy</span><strong>{(selectedTaskState?.entropy ?? 0).toFixed(3)}</strong></div>
                        <div className="surface-stat"><span>Collapse</span><strong>{((selectedTaskState?.collapseProbability ?? 0) * 100).toFixed(1)}%</strong></div>
                        <div className="surface-stat"><span>Steps</span><strong>{selectedTaskState?.stepCount ?? 0}</strong></div>
                      </div>
                      <div className="surface-actions">
                        <button type="button" className="btn-sm accent" onClick={focusSelectedTask}>Apply Vector</button>
                        <button type="button" className="btn-sm" onClick={pulseSelectedTask}>Pulse</button>
                        <button type="button" className="btn-sm" onClick={stabilizeSelectedTask}>Stabilize</button>
                        <button type="button" className="btn-sm danger" onClick={hardCollapseSelectedTask}>Collapse</button>
                      </div>
                    </>
                  ) : (
                    <p className="empty">Select a task to expose the engine controls.</p>
                  )}
                </div>

                <div className="panel-block">
                  <div className="panel-title">physics models</div>
                  <div className="phase-model-row">
                    {Object.entries(activeModels).map(([model, on]) => (
                      <button key={model} type="button"
                        className={`model-pill ${on ? 'on' : 'off'}`}
                        onClick={() => setActiveModels((prev) => ({ ...prev, [model]: !prev[model] }))}>
                        {model}
                      </button>
                    ))}
                  </div>
                  <DampingControls
                    isDampingEnabled={isDampingEnabled}
                    dampingCoefficient={dampingCoefficient}
                    onEnableDamping={(coeff) => { setIsDampingEnabled(true); setDampingCoefficient(coeff); }}
                    onDisableDamping={() => setIsDampingEnabled(false)}
                  />
                </div>

                {selectedTask && !selectedTask.completed && (
                  <div className="panel-block task-detail">
                    <div className="detail-name">{selectedTask.title}</div>
                    <div className="detail-row">
                      <span className="detail-key">GROUP</span>
                      <span className="detail-val" style={{ color: (groupMap[selectedTask.groupId] ?? DEFAULT_GROUP).color }}>
                        {(groupMap[selectedTask.groupId] ?? DEFAULT_GROUP).name}
                      </span>
                    </div>
                    <div className="detail-row">
                      <span className="detail-key">DIFFICULTY</span>
                      <span className="detail-val">{selectedTask.difficulty}</span>
                    </div>
                    <div className="detail-row">
                      <span className="detail-key">ENTROPY</span>
                      <span className="detail-val">{(visibleTaskStates[selectedTask.id]?.entropy ?? 0).toFixed(3)}</span>
                    </div>
                    <div className="detail-row">
                      <span className="detail-key">STRESS X</span>
                      <span className="detail-val">{(visibleTaskStates[selectedTask.id]?.stressX ?? 0).toFixed(3)}</span>
                    </div>
                    <div className="detail-row">
                      <span className="detail-key">STRESS Y</span>
                      <span className="detail-val">{(visibleTaskStates[selectedTask.id]?.stressY ?? 0).toFixed(3)}</span>
                    </div>
                    <div className="detail-row">
                      <span className="detail-key">COLLAPSE P</span>
                      <span className="detail-val">{((visibleTaskStates[selectedTask.id]?.collapseProbability ?? 0) * 100).toFixed(1)}%</span>
                    </div>
                    <div className="detail-row">
                      <span className="detail-key">STEPS</span>
                      <span className="detail-val">{visibleTaskStates[selectedTask.id]?.stepCount ?? 0}</span>
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

                {/* System heat bar */}
                <div className="panel-block">
                  <div className="panel-title">thermal state</div>
                  <div className="heat-bar-wrap">
                    <div className="heat-bar-track">
                      <div className="heat-bar-fill" style={{ width: `${heatPct}%`, background: heatPct > 80 ? 'linear-gradient(90deg,var(--amber),var(--coral))' : 'linear-gradient(90deg,var(--green),var(--amber))' }}/>
                    </div>
                    <div className="heat-labels">
                      <span>{heatDisplay}K</span>
                      <span style={{ color: heatPct > 80 ? 'var(--coral)' : 'var(--muted)' }}>{heatPct > 80 ? '🔥 CRITICAL' : 'NOMINAL'}</span>
                    </div>
                  </div>
                </div>
              </aside>
            </div>
          }/>

          {/* ── TASKS PAGE ── */}
          <Route path="/tasks" element={
            <div className="tasks-page">
              <div className="tasks-header">
                <div className="tasks-header-left">
                  <h2>Task Board</h2>
                  <span className="tasks-count">{activeTasks.length} active · {completedTasks.length} done</span>
                </div>
                <form className="task-inline-form" onSubmit={addTask}>
                  <input type="text" value={newTaskTitle} onChange={(e) => setNewTaskTitle(e.target.value)} placeholder="New task title..." className="mini-input" />
                  <select className="mini-input compact" value={newTaskGroupId} onChange={(e) => setNewTaskGroupId(e.target.value)}>
                    {groups.map((g) => <option key={g.id} value={g.id}>{g.name}</option>)}
                  </select>
                  <input type="range" min={1} max={10} value={newTaskDifficulty} onChange={(e) => setNewTaskDifficulty(Number(e.target.value))} title={`Difficulty: ${newTaskDifficulty}`} />
                  <span className="difficulty-badge">D:{newTaskDifficulty}</span>
                  <button className="btn-primary compact" type="submit">+ Add</button>
                </form>
              </div>
              <div className="group-board">
                {groupedTasks.map(({ group, tasks: grouped }) => {
                  const active = grouped.filter(t => !t.completed);
                  const done = grouped.filter(t => t.completed);
                  return (
                    <section key={group.id} className={`group-column ${dragTaskId ? 'drop-ready' : ''}`}
                      onDragOver={(e) => e.preventDefault()}
                      onDrop={(e) => { e.preventDefault(); if (!dragTaskId) return; moveTask(dragTaskId, group.id); setDragTaskId(null); }}>
                      <div className="group-col-header">
                        <span className="group-swatch" style={{ background: group.color }}/>
                        <h3 style={{ color: group.color }}>{group.name}</h3>
                        <span className="group-count">{active.length}</span>
                      </div>
                      {active.length === 0 && done.length === 0 && <p className="empty">Drop tasks here</p>}
                      {active.map((task) => {
                        const state = visibleTaskStates[task.id];
                        return (
                          <div key={task.id} className="draggable-task" draggable
                            onDragStart={(e) => { e.dataTransfer.effectAllowed = 'move'; setDragTaskId(task.id); }}
                            onDragEnd={() => setDragTaskId(null)}
                            onDragOver={(e) => e.preventDefault()}
                            onDrop={(e) => { e.preventDefault(); if (!dragTaskId) return; moveTask(dragTaskId, group.id, task.id); setDragTaskId(null); }}>
                            <div className="task-card-top">
                              <button type="button" className="task-check-card" onClick={() => toggleComplete(task.id)} title="Mark complete">
                                <span className="check-inner-card"></span>
                              </button>
                              <div className="task-list-title">{task.title}</div>
                            </div>
                            <div className="task-physics-row">
                              <span className="phys-badge">D:{task.difficulty}</span>
                              {state && <span className="phys-badge purple">S:{state.entropy.toFixed(1)}</span>}
                              {state && <span className="phys-badge amber">P:{(state.collapseProbability * 100).toFixed(0)}%</span>}
                            </div>
                            <div className="difficulty-controls" role="group">
                              {[1,2,3,4,5,6,7,8,9,10].map((level) => (
                                <button type="button" key={level}
                                  className={task.difficulty === level ? 'level active' : 'level'}
                                  onClick={() => patchTask(task.id, { difficulty: level })}>{level}</button>
                              ))}
                            </div>
                            <div className="energy-bar-wrap" style={{ marginTop: 6 }}>
                              <div className="energy-bar" style={{ width: `${Math.min(100, task.difficulty * 10)}%`, background: group.color }}/>
                            </div>
                            <div className="task-actions">
                              <button type="button" className="btn-sm" onClick={() => { setSelectedTaskId(task.id); navigate('/simulation'); }}>⚛ Simulate</button>
                              <button type="button" className="btn-sm accent-btn" onClick={() => toggleComplete(task.id)}>✓ Done</button>
                              <button type="button" className="btn-sm danger" onClick={() => removeTask(task.id)}>✕</button>
                            </div>
                          </div>
                        );
                      })}
                      {done.length > 0 && (
                        <div className="completed-section">
                          <div className="completed-divider">✓ {done.length} completed</div>
                          {done.map((task) => (
                            <div key={task.id} className="draggable-task completed-card">
                              <div className="task-card-top">
                                <button type="button" className="task-check-card checked" onClick={() => toggleComplete(task.id)}>
                                  <span className="check-inner-card">✓</span>
                                </button>
                                <div className="task-list-title completed-name">{task.title}</div>
                              </div>
                              <div className="task-actions">
                                <button type="button" className="btn-sm" onClick={() => toggleComplete(task.id)}>↩ Restore</button>
                                <button type="button" className="btn-sm danger" onClick={() => removeTask(task.id)}>✕</button>
                              </div>
                            </div>
                          ))}
                        </div>
                      )}
                    </section>
                  );
                })}
              </div>
            </div>
          }/>

          {/* ── GROUPS PAGE ── */}
          <Route path="/groups" element={
            <div className="groups-page">
              <div className="groups-left">
                <div className="panel-card">
                  <h2>Create Group</h2>
                  <form className="task-form" onSubmit={addGroup}>
                    <input type="text" value={newGroupName} onChange={(e) => setNewGroupName(e.target.value)} placeholder="Group name" />
                    <div className="inline-row">
                      <label htmlFor="group-color">Color</label>
                      <input id="group-color" type="color" value={newGroupColor} onChange={(e) => setNewGroupColor(e.target.value)} />
                    </div>
                    <button className="btn-primary" type="submit">Add Group</button>
                  </form>
                </div>
                <div className="panel-card">
                  <h2>Groups</h2>
                  <div className="group-list">
                    {groupedTasks.map(({ group, tasks: grouped }) => {
                      const active = grouped.filter(t => !t.completed);
                      const done = grouped.filter(t => t.completed);
                      return (
                        <div className="group-item" key={group.id}>
                          <div className="group-item-left">
                            <span className="swatch" style={{ background: group.color }}/>
                            <div>
                              <div className="group-title">{group.name}</div>
                              <div className="group-stats">{active.length} active · {done.length} done</div>
                            </div>
                          </div>
                          <button type="button" className="btn-sm danger" onClick={() => deleteGroup(group.id)} disabled={group.id === DEFAULT_GROUP_ID}>Delete</button>
                        </div>
                      );
                    })}
                  </div>
                </div>
              </div>
              <div className="groups-right">
                <div className="panel-card full-height">
                  <h2>Group Distribution</h2>
                  <svg width="100%" height="300" viewBox="0 0 300 300">
                    {(() => {
                      const total = tasks.length || 1;
                      let angle = -Math.PI / 2;
                      return groupedTasks.filter(({ tasks: t }) => t.length > 0).map(({ group, tasks: t }) => {
                        const slice = (t.length / total) * Math.PI * 2;
                        const x1 = 150 + 100 * Math.cos(angle);
                        const y1 = 150 + 100 * Math.sin(angle);
                        angle += slice;
                        const x2 = 150 + 100 * Math.cos(angle);
                        const y2 = 150 + 100 * Math.sin(angle);
                        const large = slice > Math.PI ? 1 : 0;
                        const midAngle = angle - slice / 2;
                        const lx = 150 + 120 * Math.cos(midAngle);
                        const ly = 150 + 120 * Math.sin(midAngle);
                        return (
                          <g key={group.id}>
                            <path d={`M150,150 L${x1},${y1} A100,100 0 ${large},1 ${x2},${y2} Z`} fill={group.color} opacity="0.8"/>
                            <text x={lx} y={ly} textAnchor="middle" fontSize="10" fill="white" opacity="0.9">{group.name.slice(0,8)}</text>
                          </g>
                        );
                      });
                    })()}
                    {tasks.length === 0 && <text x="150" y="155" textAnchor="middle" fill="rgba(255,255,255,0.3)" fontSize="12">No tasks yet</text>}
                  </svg>
                  <div className="group-legend">
                    {groupedTasks.map(({ group, tasks: t }) => (
                      <div key={group.id} className="legend-item">
                        <span className="legend-dot" style={{ background: group.color }}/>
                        <span>{group.name}</span>
                        <span className="legend-count">{t.length}</span>
                      </div>
                    ))}
                  </div>
                </div>
              </div>
            </div>
          }/>

          {/* ── ANALYTICS PAGE ── */}
          <Route path="/analytics" element={
            <div className="analytics-page">
              <div className="analytics-grid">
                <div className="panel-card">
                  <h2>System Heat</h2>
                  <p className="big-stat" style={{ color: heatPct > 80 ? 'var(--coral)' : 'var(--amber)' }}>{heatDisplay} K</p>
                  <div className="bar"><div className="fill" style={{ width: `${heatPct}%`, background: heatPct > 80 ? 'var(--coral)' : 'var(--amber)' }}/></div>
                  <p className="stat-sub">{heatPct > 80 ? '🔥 Approaching burnout' : '✓ Nominal operating range'}</p>
                </div>
                <div className="panel-card">
                  <h2>Average Difficulty</h2>
                  <p className="big-stat accent">{avgDifficulty.toFixed(2)}</p>
                  <p className="stat-sub">Std Dev: {stdDevEnergy.toFixed(2)} eV</p>
                </div>
                <div className="panel-card">
                  <h2>Completion Rate</h2>
                  <p className="big-stat green">{tasks.length ? ((completedTasks.length / tasks.length) * 100).toFixed(0) : 0}%</p>
                  <div className="bar"><div className="fill green-fill" style={{ width: `${tasks.length ? (completedTasks.length / tasks.length) * 100 : 0}%` }}/></div>
                  <p className="stat-sub">{completedTasks.length} of {tasks.length} tasks done</p>
                </div>
                <div className="panel-card">
                  <h2>Total Entropy</h2>
                  <p className="big-stat purple">{totalEntropy.toFixed(2)}</p>
                  <p className="stat-sub">Avg per task: {entropyScore.toFixed(3)}</p>
                </div>
                <EnergyAnalytics meanEnergy={totalEnergy / Math.max(activeTasks.length, 1)} stdDevEnergy={stdDevEnergy} />
                <div className="panel-card">
                  <h2>Group Balance</h2>
                  <ul className="stat-list">
                    {groupedTasks.map(({ group, tasks: grouped }) => {
                      const active = grouped.filter(t => !t.completed);
                      const done = grouped.filter(t => t.completed);
                      return (
                        <li key={group.id}>
                          <span className="legend-dot" style={{ background: group.color }}/>
                          <span>{group.name}</span>
                          <span className="stat-count">{active.length} active</span>
                          <span className="stat-done">{done.length} done</span>
                        </li>
                      );
                    })}
                  </ul>
                </div>
                <div className="panel-card span-2">
                  <h2>Energy Timeline</h2>
                  <svg width="100%" height="120" viewBox="0 0 600 120" preserveAspectRatio="none">
                    <defs>
                      <linearGradient id="energy-grad" x1="0" y1="0" x2="0" y2="1">
                        <stop offset="0%" stopColor="var(--accent)" stopOpacity="0.4"/>
                        <stop offset="100%" stopColor="var(--accent)" stopOpacity="0"/>
                      </linearGradient>
                      <linearGradient id="entropy-grad" x1="0" y1="0" x2="0" y2="1">
                        <stop offset="0%" stopColor="var(--accent2)" stopOpacity="0.4"/>
                        <stop offset="100%" stopColor="var(--accent2)" stopOpacity="0"/>
                      </linearGradient>
                    </defs>
                    <path d={sparklinePath(energyHistory, 600, 110) + ' L600,120 L0,120 Z'} fill="url(#energy-grad)"/>
                    <path d={sparklinePath(energyHistory, 600, 110)} fill="none" stroke="var(--accent)" strokeWidth="2"/>
                    <path d={sparklinePath(entropyHistory, 600, 110) + ' L600,120 L0,120 Z'} fill="url(#entropy-grad)"/>
                    <path d={sparklinePath(entropyHistory, 600, 110)} fill="none" stroke="var(--accent2)" strokeWidth="2" strokeDasharray="4 2"/>
                  </svg>
                  <div className="chart-legend">
                    <span><span className="legend-dot" style={{ background: 'var(--accent)' }}/> Energy</span>
                    <span><span className="legend-dot" style={{ background: 'var(--accent2)' }}/> Entropy</span>
                  </div>
                </div>
                <div className="panel-card">
                  <h2>Physics Models</h2>
                  <div className="model-status-list">
                    {Object.entries(activeModels).map(([model, on]) => (
                      <div key={model} className="model-status-row">
                        <span className={`model-status-dot ${on ? 'on' : 'off'}`}/>
                        <span className="model-status-name">{model}</span>
                        <span className="model-status-val">{on ? 'ACTIVE' : 'IDLE'}</span>
                      </div>
                    ))}
                  </div>
                </div>
              </div>
            </div>
          }/>

          <Route path="*" element={<Navigate to="/simulation" replace />} />
        </Routes>
      </div>
    </div>
  );
}

export default App;
