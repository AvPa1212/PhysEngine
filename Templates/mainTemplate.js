import { initSimulation, stepSimulation } from "./utils/simulationBridge.js";

const canvas = document.getElementById("simulationCanvas");
const ctx = canvas.getContext("2d");

let tasks = [];
let running = true;

function resizeCanvas() {
  canvas.width = canvas.clientWidth;
  canvas.height = canvas.clientHeight;
}

window.addEventListener("resize", resizeCanvas);
resizeCanvas();

function render() {
  ctx.clearRect(0, 0, canvas.width, canvas.height);

  tasks.forEach(task => {
    ctx.beginPath();
    ctx.arc(task.x, task.y, 6, 0, Math.PI * 2);
    ctx.fillStyle = "#3b82f6";
    ctx.fill();
  });
}

function loop() {
  if (running) {
    tasks = stepSimulation(tasks);
    render();
  }
  requestAnimationFrame(loop);
}

document.querySelector("button").onclick = () => {
  tasks.push({
    id: crypto.randomUUID(),
    x: Math.random() * canvas.width,
    y: Math.random() * canvas.height,
    vx: 0,
    vy: 0,
    mass: 1
  });
};

loop();