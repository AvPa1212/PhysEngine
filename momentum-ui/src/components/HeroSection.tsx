import React, { useEffect, useRef, useState } from 'react';
import PhysicsOrb from './PhysicsOrb';

/**
 * Computes the translate offset for a Physics Orb based on pointer proximity.
 *
 * When the pointer is within 120 px of the orb center, returns an offset
 * proportional to the distance, clamped to ±24 px per axis.
 * When the pointer is outside 120 px, returns { offsetX: 0, offsetY: 0 }.
 *
 * Exported for unit/property testing.
 */
export function computeOrbOffset(
  pointerX: number,
  pointerY: number,
  orbCenterX: number,
  orbCenterY: number
): { offsetX: number; offsetY: number } {
  const dx = pointerX - orbCenterX;
  const dy = pointerY - orbCenterY;
  const dist = Math.sqrt(dx * dx + dy * dy);

  if (dist > 120 || dist === 0) {
    return { offsetX: 0, offsetY: 0 };
  }

  // Scale: closer pointer → larger offset, max ±24 px
  const scale = (1 - dist / 120) * 24;
  const offsetX = Math.max(-24, Math.min(24, (dx / dist) * scale));
  const offsetY = Math.max(-24, Math.min(24, (dy / dist) * scale));

  return { offsetX, offsetY };
}

interface OrbOffset {
  offsetX: number;
  offsetY: number;
}

const ORB_CONFIGS: Array<{
  size: number;
  color: 'accent' | 'accent-2' | 'accent-3';
  style: React.CSSProperties;
  animationDelay: string;
}> = [
  {
    size: 120,
    color: 'accent',
    style: { position: 'absolute', top: '15%', left: '10%' },
    animationDelay: '0s',
  },
  {
    size: 80,
    color: 'accent-2',
    style: { position: 'absolute', top: '60%', right: '12%' },
    animationDelay: '0.6s',
  },
  {
    size: 100,
    color: 'accent-3',
    style: { position: 'absolute', bottom: '20%', left: '55%' },
    animationDelay: '1.2s',
  },
];

// Approximate orb center positions (relative to section) for offset computation
const ORB_CENTERS = [
  { xFrac: 0.1, yFrac: 0.15 },
  { xFrac: 0.88, yFrac: 0.6 },
  { xFrac: 0.55, yFrac: 0.8 },
];

const HeroSection: React.FC = () => {
  const sectionRef = useRef<HTMLElement>(null);
  const pointerRef = useRef<{ x: number; y: number }>({ x: -9999, y: -9999 });
  const rafRef = useRef<number>(0);
  const [orbOffsets, setOrbOffsets] = useState<OrbOffset[]>([
    { offsetX: 0, offsetY: 0 },
    { offsetX: 0, offsetY: 0 },
    { offsetX: 0, offsetY: 0 },
  ]);

  const reducedMotion =
    typeof window !== 'undefined' &&
    window.matchMedia('(prefers-reduced-motion: reduce)').matches;

  const hoverCapable =
    typeof window !== 'undefined' &&
    window.matchMedia('(hover: hover)').matches;

  useEffect(() => {
    if (reducedMotion || !hoverCapable) return;

    const section = sectionRef.current;
    if (!section) return;

    const handlePointerMove = (e: PointerEvent) => {
      const rect = section.getBoundingClientRect();
      pointerRef.current = {
        x: e.clientX - rect.left,
        y: e.clientY - rect.top,
      };
    };

    const loop = () => {
      const { x: px, y: py } = pointerRef.current;
      const rect = section.getBoundingClientRect();
      const w = rect.width;
      const h = rect.height;

      const newOffsets = ORB_CENTERS.map((center) => {
        const cx = center.xFrac * w;
        const cy = center.yFrac * h;
        return computeOrbOffset(px, py, cx, cy);
      });

      setOrbOffsets(newOffsets);
      rafRef.current = requestAnimationFrame(loop);
    };

    section.addEventListener('pointermove', handlePointerMove);
    rafRef.current = requestAnimationFrame(loop);

    return () => {
      section.removeEventListener('pointermove', handlePointerMove);
      cancelAnimationFrame(rafRef.current);
    };
  }, [reducedMotion, hoverCapable]);

  return (
    <section
      ref={sectionRef}
      style={{
        position: 'relative',
        height: '100vh',
        display: 'flex',
        flexDirection: 'column',
        alignItems: 'center',
        justifyContent: 'center',
        overflow: 'hidden',
      }}
    >
      {/* Physics Orbs */}
      {ORB_CONFIGS.map((orb, i) => (
        <PhysicsOrb
          key={i}
          size={orb.size}
          color={orb.color}
          offsetX={reducedMotion ? 0 : orbOffsets[i]?.offsetX ?? 0}
          offsetY={reducedMotion ? 0 : orbOffsets[i]?.offsetY ?? 0}
          style={{
            ...orb.style,
            animation: reducedMotion
              ? 'none'
              : `float 3s ease-in-out infinite alternate, pulse 4s ease-in-out infinite`,
            animationDelay: reducedMotion ? '0s' : orb.animationDelay,
          }}
        />
      ))}

      {/* Heading */}
      <h1
        style={{
          fontFamily: 'var(--mono)',
          fontSize: 'clamp(2.4rem, 8vw, 6rem)',
          letterSpacing: '0.05em',
          margin: 0,
          position: 'relative',
          zIndex: 1,
        }}
      >
        MOMENTUM
      </h1>

      {/* Tagline */}
      <p
        style={{
          fontFamily: 'var(--sans)',
          fontSize: 'clamp(1rem, 2vw, 1.25rem)',
          marginTop: '1rem',
          opacity: 0.75,
          textAlign: 'center',
          maxWidth: '480px',
          position: 'relative',
          zIndex: 1,
        }}
      >
        Physics-based task management — where every task obeys the laws of motion.
      </p>

      {/* Scroll affordance chevron */}
      <div
        aria-label="Scroll to explore"
        style={{
          position: 'absolute',
          bottom: '2rem',
          left: '50%',
          transform: 'translateX(-50%)',
          animation: reducedMotion ? 'none' : 'float 2s ease-in-out infinite alternate',
          zIndex: 1,
        }}
      >
        <svg
          aria-hidden="true"
          width="32"
          height="32"
          viewBox="0 0 24 24"
          fill="none"
          stroke="currentColor"
          strokeWidth="2"
          strokeLinecap="round"
          strokeLinejoin="round"
          style={{ opacity: 0.6 }}
        >
          <polyline points="6 9 12 15 18 9" />
        </svg>
      </div>
    </section>
  );
};

export default HeroSection;
