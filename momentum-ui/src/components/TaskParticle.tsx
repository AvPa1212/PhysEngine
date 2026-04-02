import React from 'react';

type TaskParticleProps = {
  x: number;
  y: number;
  vx: number;
  vy: number;
  id: string | number;
};

const SCALE_FACTOR = 5;
const PARTICLE_RADIUS = 10;
const TRAIL_LENGTH = 30;

function TaskParticle({ x, y, vx, vy, id }: TaskParticleProps) {
  const screenX = x * SCALE_FACTOR;
  const screenY = y * SCALE_FACTOR;

  const speed = Math.sqrt(vx * vx + vy * vy);
  const hasVelocity = speed > 0.001;

  const trailDx = hasVelocity ? (vx / speed) * TRAIL_LENGTH : 0;
  const trailDy = hasVelocity ? (vy / speed) * TRAIL_LENGTH : 0;

  return (
    <g
      data-testid={`task-particle-${id}`}
      transform={`translate(${screenX}, ${screenY})`}
      style={{ transition: 'transform 0.016s linear' }}
    >
      {hasVelocity && (
        <line
          data-testid={`task-particle-trail-${id}`}
          x1={0}
          y1={0}
          x2={-trailDx}
          y2={-trailDy}
          stroke="rgba(79, 142, 247, 0.5)"
          strokeWidth={2}
          strokeLinecap="round"
        />
      )}

      <circle
        data-testid={`task-particle-circle-${id}`}
        cx={0}
        cy={0}
        r={PARTICLE_RADIUS}
        fill="rgba(79, 142, 247, 0.8)"
        stroke="rgba(79, 142, 247, 1)"
        strokeWidth={1.5}
      />

      {hasVelocity && (
        <circle
          cx={trailDx}
          cy={trailDy}
          r={3}
          fill="rgba(79, 142, 247, 0.9)"
        />
      )}
    </g>
  );
}

export default TaskParticle;