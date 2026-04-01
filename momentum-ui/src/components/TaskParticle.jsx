import React from 'react';

/**
 * TaskParticle – Renders a single task as a circle in 2D simulation space.
 *
 * Scales simulation coordinates to screen space (multiply by SCALE_FACTOR),
 * and displays a velocity arrow/trail showing the direction of motion.
 *
 * Props:
 *   x  – simulation-space X position
 *   y  – simulation-space Y position
 *   vx – X velocity component
 *   vy – Y velocity component
 *   id – unique task identifier (string or number)
 *
 * Requirements: 10.4, 10.5
 */

const SCALE_FACTOR = 5;
const PARTICLE_RADIUS = 10;
const TRAIL_LENGTH = 30;

function TaskParticle({ x, y, vx, vy, id }) {
  // Scale from simulation space to screen space
  const screenX = x * SCALE_FACTOR;
  const screenY = y * SCALE_FACTOR;

  // Compute velocity magnitude for trail scaling
  const speed = Math.sqrt(vx * vx + vy * vy);
  const hasVelocity = speed > 0.001;

  // Normalised velocity direction for the trail arrow
  const trailDx = hasVelocity ? (vx / speed) * TRAIL_LENGTH : 0;
  const trailDy = hasVelocity ? (vy / speed) * TRAIL_LENGTH : 0;

  return (
    <g
      data-testid={`task-particle-${id}`}
      transform={`translate(${screenX}, ${screenY})`}
      style={{ transition: 'transform 0.016s linear' }}
    >
      {/* Velocity trail */}
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

      {/* Particle body */}
      <circle
        data-testid={`task-particle-circle-${id}`}
        cx={0}
        cy={0}
        r={PARTICLE_RADIUS}
        fill="rgba(79, 142, 247, 0.8)"
        stroke="rgba(79, 142, 247, 1)"
        strokeWidth={1.5}
      />

      {/* Velocity arrow head */}
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
