import React, { useEffect, useRef, useState } from 'react';
import PhysicsOrb from './PhysicsOrb';

/**
 * Computes the translate offset for a Physics Orb based on pointer proximity.
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
  floatDuration: string;
}> = [
  {
    size: 160,
    color: 'accent',
    style: { position: 'absolute', top: '12%', left: '8%', opacity: 0.55 },
    animationDelay: '0s',
    floatDuration: '6s',
  },
  {
    size: 100,
    color: 'accent-2',
    style: { position: 'absolute', top: '55%', right: '10%', opacity: 0.45 },
    animationDelay: '1.2s',
    floatDuration: '8s',
  },
  {
    size: 130,
    color: 'accent-3',
    style: { position: 'absolute', bottom: '18%', left: '52%', opacity: 0.4 },
    animationDelay: '2.4s',
    floatDuration: '7s',
  },
];

const ORB_CENTERS = [
  { xFrac: 0.08, yFrac: 0.12 },
  { xFrac: 0.9, yFrac: 0.55 },
  { xFrac: 0.52, yFrac: 0.82 },
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
  const [mounted, setMounted] = useState(false);

  const reducedMotion =
    typeof window !== 'undefined' &&
    window.matchMedia('(prefers-reduced-motion: reduce)').matches;

  const hoverCapable =
    typeof window !== 'undefined' &&
    window.matchMedia('(hover: hover)').matches;

  // Entrance animation trigger
  useEffect(() => {
    const t = setTimeout(() => setMounted(true), 60);
    return () => clearTimeout(t);
  }, []);

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
      {/* Aurora background glow */}
      <div aria-hidden="true" style={{
        position: 'absolute',
        inset: 0,
        background: `
          radial-gradient(ellipse 60% 40% at 20% 30%, rgba(109,167,255,0.18) 0%, transparent 60%),
          radial-gradient(ellipse 50% 35% at 80% 60%, rgba(155,140,255,0.15) 0%, transparent 55%),
          radial-gradient(ellipse 40% 30% at 50% 90%, rgba(72,214,201,0.12) 0%, transparent 50%)
        `,
        animation: reducedMotion ? 'none' : 'aurora 12s ease-in-out infinite alternate',
        pointerEvents: 'none',
      }} />

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
              : `heroFloat ${orb.floatDuration} ease-in-out infinite alternate`,
            animationDelay: reducedMotion ? '0s' : orb.animationDelay,
            transition: reducedMotion ? 'none' : 'transform 0.12s ease-out',
          }}
        />
      ))}

      {/* Heading */}
      <h1
        style={{
          fontFamily: 'var(--mono)',
          fontSize: 'clamp(2.4rem, 8vw, 6rem)',
          letterSpacing: '0.06em',
          margin: 0,
          position: 'relative',
          zIndex: 1,
          background: 'linear-gradient(135deg, var(--text) 30%, var(--accent) 70%, var(--accent-2) 100%)',
          WebkitBackgroundClip: 'text',
          WebkitTextFillColor: 'transparent',
          backgroundClip: 'text',
          opacity: reducedMotion ? 1 : (mounted ? 1 : 0),
          transform: reducedMotion ? 'none' : (mounted ? 'translateY(0)' : 'translateY(20px)'),
          transition: reducedMotion ? 'none' : 'opacity 0.8s ease, transform 0.8s ease',
        }}
      >
        MOMENTUM
      </h1>

      {/* Tagline */}
      <p
        style={{
          fontFamily: 'var(--sans)',
          fontSize: 'clamp(1rem, 2vw, 1.2rem)',
          marginTop: '1.25rem',
          opacity: reducedMotion ? 0.7 : (mounted ? 0.7 : 0),
          textAlign: 'center',
          maxWidth: '520px',
          position: 'relative',
          zIndex: 1,
          lineHeight: 1.6,
          transform: reducedMotion ? 'none' : (mounted ? 'translateY(0)' : 'translateY(16px)'),
          transition: reducedMotion ? 'none' : 'opacity 0.8s ease 0.2s, transform 0.8s ease 0.2s',
        }}
      >
        Physics-based task management — where every task obeys the laws of motion.
      </p>

      {/* Stat pills */}
      <div style={{
        display: 'flex',
        gap: '12px',
        marginTop: '2.5rem',
        flexWrap: 'wrap',
        justifyContent: 'center',
        position: 'relative',
        zIndex: 1,
        opacity: reducedMotion ? 1 : (mounted ? 1 : 0),
        transform: reducedMotion ? 'none' : (mounted ? 'translateY(0)' : 'translateY(12px)'),
        transition: reducedMotion ? 'none' : 'opacity 0.8s ease 0.4s, transform 0.8s ease 0.4s',
      }}>
        {[
          { label: '5 Physics Models', icon: '⚛' },
          { label: 'WebAssembly Core', icon: '⚡' },
          { label: 'Real-time Simulation', icon: '◉' },
        ].map(({ label, icon }) => (
          <div key={label} className="sim-badge" style={{ gap: '8px' }}>
            <span aria-hidden="true">{icon}</span>
            <span style={{ fontFamily: 'var(--mono)', fontSize: '0.78rem' }}>{label}</span>
          </div>
        ))}
      </div>

      {/* Scroll affordance */}
      <div
        aria-label="Scroll to explore"
        style={{
          position: 'absolute',
          bottom: '2rem',
          left: '50%',
          transform: 'translateX(-50%)',
          animation: reducedMotion ? 'none' : 'float 2s ease-in-out infinite alternate',
          zIndex: 1,
          opacity: 0.5,
        }}
      >
        <svg
          aria-hidden="true"
          width="28"
          height="28"
          viewBox="0 0 24 24"
          fill="none"
          stroke="currentColor"
          strokeWidth="1.5"
          strokeLinecap="round"
          strokeLinejoin="round"
        >
          <polyline points="6 9 12 15 18 9" />
        </svg>
      </div>

      <style>{`
        @keyframes heroFloat {
          from { transform: translateY(0px) scale(1); }
          to   { transform: translateY(-18px) scale(1.04); }
        }
      `}</style>
    </section>
  );
};

export default HeroSection;
