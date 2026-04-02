import { useEffect, useRef } from "react";

export default function Canvas({ tasks }) {
  const ref = useRef(null);

  useEffect(() => {
    const canvas = ref.current;
    const ctx = canvas.getContext("2d");

    ctx.clearRect(0, 0, canvas.width, canvas.height);

    tasks.forEach(task => {
      ctx.beginPath();
      ctx.arc(task.x, task.y, 5, 0, Math.PI * 2);
      ctx.fill();
    });
  }, [tasks]);

  return <canvas ref={ref} className="w-full h-full" />;
}