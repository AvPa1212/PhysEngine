export function initSimulation() {
  // Future: initialize WASM engine or API connection
}

export function stepSimulation(tasks) {
  // Placeholder physics (replace with backend output)

  return tasks.map(t => {
    return {
      ...t,
      x: t.x + t.vx,
      y: t.y + t.vy
    };
  });
}