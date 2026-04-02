export type HydratedTask = {
  id: string;
  title: string;
  difficulty: number;
  groupId: string;
  createdAt: number;
  completed: boolean;
};

export type HydratedGroup = {
  id: string;
  name: string;
  color: string;
};

export function clamp(value: number, min: number, max: number) {
  return Math.min(max, Math.max(min, value));
}

export function readJson<T>(storageKey: string): T | null {
  try {
    const raw = localStorage.getItem(storageKey);
    return raw ? JSON.parse(raw) as T : null;
  } catch {
    return null;
  }
}

export function persistJson(storageKey: string, value: unknown) {
  try {
    localStorage.setItem(storageKey, JSON.stringify(value));
  } catch {
    // localStorage can be blocked or unavailable in some environments.
  }
}

export function hydrateGroups(
  value: unknown,
  defaultGroup: HydratedGroup,
  defaultGroupId: string
): HydratedGroup[] {
  if (!Array.isArray(value)) {
    return [defaultGroup];
  }

  const hydrated = value.filter(
    (group): group is HydratedGroup =>
      Boolean(group) && typeof group.id === 'string' && typeof group.name === 'string'
  );

  return hydrated.some((group) => group.id === defaultGroupId)
    ? hydrated
    : [defaultGroup, ...hydrated];
}

export function hydrateTasks(value: unknown, defaultGroupId: string): HydratedTask[] {
  if (!Array.isArray(value)) {
    return [];
  }

  return value
    .filter((task): task is Record<string, unknown> => Boolean(task) && typeof task === 'object')
    .filter((task) => Boolean(task.title))
    .map((task) => ({
      id: String(task.id),
      title: String(task.title),
      difficulty: clamp(Number(task.difficulty) || 1, 1, 10),
      groupId: task.groupId ? String(task.groupId) : defaultGroupId,
      createdAt: Number(task.createdAt) || Date.now(),
      completed: Boolean(task.completed),
    }));
}
