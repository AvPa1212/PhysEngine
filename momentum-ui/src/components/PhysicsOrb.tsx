import React from 'react';

interface PhysicsOrbProps {
  size: number;
  color: 'accent' | 'accent-2' | 'accent-3';
  style?: React.CSSProperties;
  offsetX?: number;
  offsetY?: number;
}

const PhysicsOrb: React.FC<PhysicsOrbProps> = ({
  size,
  color,
  style,
  offsetX = 0,
  offsetY = 0,
}) => {
  const gradientId = `orb-gradient-${color}-${size}`;
  const cssVar = `--${color}`;
  const radius = size / 2;

  return (
    <svg
      aria-hidden="true"
      width={size}
      height={size}
      viewBox={`0 0 ${size} ${size}`}
      style={{
        transform: `translate(${offsetX}px, ${offsetY}px)`,
        ...style,
      }}
    >
      <defs>
        <radialGradient id={gradientId} cx="35%" cy="35%" r="65%">
          <stop offset="0%" stopColor={`var(${cssVar})`} stopOpacity="0.9" />
          <stop offset="100%" stopColor={`var(${cssVar})`} stopOpacity="0.2" />
        </radialGradient>
      </defs>
      <circle
        cx={radius}
        cy={radius}
        r={radius}
        fill={`url(#${gradientId})`}
      />
    </svg>
  );
};

export default PhysicsOrb;
