import React, { useCallback, useEffect, useRef, useState } from 'react';

// ── Types ─────────────────────────────────────────────────────────────────────

export interface OrbState {
  id: number;
  x: number;
  y: number;
  vx: number;
  vy: number;
  r: number;
  color: 'accent' | 'accent-2' | 'accent-3';
}

// ── Pure helper functions (exported for testing) ──────────────────────────────

/**
 * Compute the repulsion impulse applied to an orb when the user taps at (tapX, tapY).
 * Δv = (dx/dist) * clamp(1/dist, 0, 12) per axis, each clamped to [-12, 12].
 */
export function computeRepulsionImpulse(
  tapX: number,
  tapY: number,
  orbX: number,
  orbY: number
): { dvx: number; dvy: number } {
  const dx = orbX - tapX;
  const dy = orbY - tapY;
  const dist = Math.sqrt(dx * dx + dy * dy);

  if (dist === 0) {
    return { dvx: 0, dvy: 0 };
  }

  const strength = Math.min(1 / dist, 12);
  const dvx = Math.max(-12, Math.min(12, (dx / dist) * strength));
  const dvy = Math.max(-12, Math.min(12, (dy / dist) * strength));

  return { dvx, dvy };
}

/**
 * Apply elastic collision response between two overlapping orbs.
 * Exchanges velocity components along the collision normal.
 * Returns updated copies of both orbs.
 */
export function applyElasticCollision(
  orb1: OrbState,
  orb2: OrbState
): { orb1: OrbState; orb2: OrbState } {
  const dx = orb2.x - orb1.x;
  const dy = orb2.y - orb1.y;
  const dist = Math.sqrt(dx * dx + dy * dy);

  if (dist === 0) {
    return { orb1, orb2 };
  }

  // Collision normal (unit vector from orb1 to orb2)
  const nx = dx / dist;
  const ny = dy / dist;

  // Project velocities onto collision normal
  const v1n = orb1.vx * nx + orb1.vy * ny;
  const v2n = orb2.vx * nx + orb2.vy * ny;

  // Only resolve if orbs are approaching each other
  if (v1n - v2n <= 0) {
    return { orb1, orb2 };
  }

  // Exchange normal velocity components (equal mass elastic collision)
  const newOrb1: OrbState = {
    ...orb1,
    vx: orb1.vx - (v1n - v2n) * nx,
    vy: orb1.vy - (v1n - v2n) * ny,
  };
  const newOrb2: OrbState = {
    ...orb2,
    vx: orb2.vx + (v1n - v2n) * nx,
    vy: orb2.vy + (v1n - v2n) * ny,
  };

  return { orb1: newOrb1, orb2: newOrb2 };
}

/**
 * Compute total kinetic energy: Σ 0.5 * r² * (vx² + vy²)
 * Returns the sum formatted to 2 decimal places as a string.
 */
export function computeKineticEnergy(orbs: OrbState[]): string {
  const total = orbs.reduce((sum, orb) => {
    return sum + 0.5 * orb.r * orb.r * (orb.vx * orb.vx + orb.vy * orb.vy);
  }, 0);
  return total.toFixed(2);
}

// ── Orb initialization ────────────────────────────────────────────────────────

const COLORS: Array<'accent' | 'accent-2' | 'accent-3'> = [
  'accent',
  'accent-2',
  'accent-3',
  'accent',
  'accent-2',
];

function createInitialOrbs(width: number, height: number): OrbState[] {
  return Array.from({ length: 5 }, (_, i) => ({
    id: i,
    x: 40 + Math.random() * (width - 80),
    y: 40 + Math.random() * (height - 80),
    vx: (Math.random() * 4 - 2),
    vy: (Math.random() * 4 - 2),
    r: 16 + Math.floor(Math.random() * 12), // 16–27 px radius
    color: COLORS[i % COLORS.length],
  }));
}

// ── Component ─────────────────────────────────────────────────────────────────

const OrbPlayground: React.FC = () => {
  const containerRef = useRef<HTMLDivElement>(null);
  const orbsRef = useRef<OrbState[]>([]);
  const rafRef = useRef<number>(0);
  const [renderOrbs, setRenderOrbs] = useState<OrbState[]>([]);
  const [keLabel, setKeLabel] = useState<string>('0.00');
  const liveRef = useRef<string>('');
  const debounceRef = useRef<ReturnType<typeof setTimeout> | null>(null);

  const reducedMotion =
    typeof window !== 'undefined' &&
    window.matchMedia('(prefers-reduced-motion: reduce)').matches;

  // Initialise orbs once the container has a size
  useEffect(() => {
    const container = containerRef.current;
    if (!container) return;

    const { width, height } = container.getBoundingClientRect();
    const w = width || 400;
    const h = height || 280;

    const initial = createInitialOrbs(w, h);
    orbsRef.current = initial;
    setRenderOrbs(initial);
    setKeLabel(computeKineticEnergy(initial));

    if (reducedMotion) return;

    const loop = () => {
      const c = containerRef.current;
      if (!c) return;
      const { width: cw, height: ch } = c.getBoundingClientRect();

      let orbs = orbsRef.current.map((orb) => ({ ...orb }));

      // 1. Update positions
      orbs = orbs.map((o) => ({ ...o, x: o.x + o.vx, y: o.y + o.vy }));

      // 2. Bounce off edges
      orbs = orbs.map((o) => {
        let { x, y, vx, vy, r } = o;
        if (x - r < 0) { x = r; vx = Math.abs(vx); }
        if (x + r > cw) { x = cw - r; vx = -Math.abs(vx); }
        if (y - r < 0) { y = r; vy = Math.abs(vy); }
        if (y + r > ch) { y = ch - r; vy = -Math.abs(vy); }
        return { ...o, x, y, vx, vy };
      });

      // 3. Elastic collisions
      for (let i = 0; i < orbs.length; i++) {
        for (let j = i + 1; j < orbs.length; j++) {
          const dx = orbs[j].x - orbs[i].x;
          const dy = orbs[j].y - orbs[i].y;
          const dist = Math.sqrt(dx * dx + dy * dy);
          if (dist < orbs[i].r + orbs[j].r) {
            const result = applyElasticCollision(orbs[i], orbs[j]);
            orbs[i] = result.orb1;
            orbs[j] = result.orb2;
          }
        }
      }

      // 4. Clamp velocities
      orbs = orbs.map((o) => ({
        ...o,
        vx: Math.max(-12, Math.min(12, o.vx)),
        vy: Math.max(-12, Math.min(12, o.vy)),
      }));

      orbsRef.current = orbs;
      setRenderOrbs([...orbs]);

      const ke = computeKineticEnergy(orbs);
      setKeLabel(ke);

      // Debounced aria-live update
      const announcement = `${orbs.length} orbs, KE ${ke}`;
      if (announcement !== liveRef.current) {
        liveRef.current = announcement;
        if (debounceRef.current) clearTimeout(debounceRef.current);
        debounceRef.current = setTimeout(() => {
          // The state update will re-render the aria-live paragraph
          setRenderOrbs((prev) => [...prev]);
        }, 500);
      }

      rafRef.current = requestAnimationFrame(loop);
    };

    rafRef.current = requestAnimationFrame(loop);

    return () => {
      cancelAnimationFrame(rafRef.current);
      if (debounceRef.current) clearTimeout(debounceRef.current);
    };
  }, [reducedMotion]);

  const handlePointerDown = useCallback((e: React.PointerEvent<HTMLDivElement>) => {
    const container = containerRef.current;
    if (!container) return;
    const rect = container.getBoundingClientRect();
    const tapX = e.clientX - rect.left;
    const tapY = e.clientY - rect.top;

    orbsRef.current = orbsRef.current.map((orb) => {
      const { dvx, dvy } = computeRepulsionImpulse(tapX, tapY, orb.x, orb.y);
      return {
        ...orb,
        vx: Math.max(-12, Math.min(12, orb.vx + dvx)),
        vy: Math.max(-12, Math.min(12, orb.vy + dvy)),
      };
    });
  }, []);

  return (
    <section
      aria-label="Physics orb playground"
      style={{ width: '100%' }}
    >
      <div
        ref={containerRef}
        onPointerDown={handlePointerDown}
        style={{
          position: 'relative',
          width: '100%',
          minHeight: 'clamp(280px, 40vw, 420px)',
          overflow: 'hidden',
          touchAction: 'manipulation',
          cursor: 'crosshair',
          borderRadius: '12px',
          background: 'rgba(255,255,255,0.03)',
          border: '1px solid rgba(255,255,255,0.08)',
        }}
      >
        <svg
          aria-hidden="true"
          style={{ position: 'absolute', inset: 0, width: '100%', height: '100%' }}
        >
          <defs>
            {(['accent', 'accent-2', 'accent-3'] as const).map((c) => (
              <radialGradient key={c} id={`orb-grad-${c}`} cx="35%" cy="35%" r="65%">
                <stop offset="0%" stopColor={`var(--${c})`} stopOpacity="0.9" />
                <stop offset="100%" stopColor={`var(--${c})`} stopOpacity="0.2" />
              </radialGradient>
            ))}
          </defs>
          {renderOrbs.map((orb) => (
            <circle
              key={orb.id}
              aria-hidden="true"
              cx={orb.x}
              cy={orb.y}
              r={orb.r}
              fill={`url(#orb-grad-${orb.color})`}
            />
          ))}
        </svg>

        {/* Real-time label */}
        <div
          style={{
            position: 'absolute',
            bottom: '8px',
            right: '12px',
            fontFamily: 'var(--mono)',
            fontSize: '0.75rem',
            opacity: 0.6,
            pointerEvents: 'none',
          }}
        >
          {renderOrbs.length} orbs · KE = {keLabel}
        </div>

        {/* Visually hidden aria-live region */}
        <p
          aria-live="polite"
          style={{
            position: 'absolute',
            width: '1px',
            height: '1px',
            padding: 0,
            margin: '-1px',
            overflow: 'hidden',
            clip: 'rect(0,0,0,0)',
            whiteSpace: 'nowrap',
            border: 0,
          }}
        >
          {liveRef.current}
        </p>
      </div>
    </section>
  );
};

export default OrbPlayground;
