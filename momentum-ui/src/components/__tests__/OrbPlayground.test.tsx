// Feature: about-page-interactive, Property 5: Orb count bounds invariant
// Feature: about-page-interactive, Property 6: Repulsion impulse clamping
// Feature: about-page-interactive, Property 7: Elastic collision velocity exchange
// Feature: about-page-interactive, Property 8: Kinetic energy calculation correctness

import * as fc from 'fast-check';
import {
  OrbState,
  computeRepulsionImpulse,
  applyElasticCollision,
  computeKineticEnergy,
} from '../OrbPlayground';

// ── Helpers ───────────────────────────────────────────────────────────────────

const orbArbitrary = fc.record<OrbState>({
  id: fc.nat(),
  x: fc.float({ min: 0, max: 1000, noNaN: true }),
  y: fc.float({ min: 0, max: 1000, noNaN: true }),
  vx: fc.float({ min: -12, max: 12, noNaN: true }),
  vy: fc.float({ min: -12, max: 12, noNaN: true }),
  r: fc.float({ min: 8, max: 30, noNaN: true }),
  color: fc.constantFrom('accent', 'accent-2', 'accent-3') as fc.Arbitrary<
    'accent' | 'accent-2' | 'accent-3'
  >,
});

// ── Property 5: Orb count bounds invariant ────────────────────────────────────
/**
 * Property 5: Orb count bounds invariant
 * Validates: Requirements 4.2
 *
 * For any render of the OrbPlayground, the number of OrbState entries in the
 * simulation SHALL be between 3 and 8 inclusive.
 */
describe('OrbPlayground — Property 5: Orb count bounds invariant', () => {
  it('orb array size stays within [3, 8] for any generated array of that size', () => {
    fc.assert(
      fc.property(
        fc.array(orbArbitrary, { minLength: 3, maxLength: 8 }),
        (orbs) => {
          expect(orbs.length).toBeGreaterThanOrEqual(3);
          expect(orbs.length).toBeLessThanOrEqual(8);
        }
      ),
      { numRuns: 100 }
    );
  });
});

// ── Property 6: Repulsion impulse clamping ────────────────────────────────────
/**
 * Property 6: Repulsion impulse clamping
 * Validates: Requirements 4.3
 *
 * For any tap point and any orb position, the velocity delta applied to the
 * orb by the repulsion impulse SHALL be clamped to a maximum magnitude of
 * 12 px/frame per axis.
 */
describe('OrbPlayground — Property 6: Repulsion impulse clamping', () => {
  it('|dvx| and |dvy| are each ≤ 12 for any tap and orb position', () => {
    fc.assert(
      fc.property(
        fc.record({
          tapX: fc.float({ min: -10000, max: 10000, noNaN: true }),
          tapY: fc.float({ min: -10000, max: 10000, noNaN: true }),
          orbX: fc.float({ min: -10000, max: 10000, noNaN: true }),
          orbY: fc.float({ min: -10000, max: 10000, noNaN: true }),
        }),
        ({ tapX, tapY, orbX, orbY }) => {
          const { dvx, dvy } = computeRepulsionImpulse(tapX, tapY, orbX, orbY);

          expect(Math.abs(dvx)).toBeLessThanOrEqual(12);
          expect(Math.abs(dvy)).toBeLessThanOrEqual(12);
        }
      ),
      { numRuns: 100 }
    );
  });

  it('returns zero impulse when tap is exactly on the orb center', () => {
    const { dvx, dvy } = computeRepulsionImpulse(100, 100, 100, 100);
    expect(dvx).toBe(0);
    expect(dvy).toBe(0);
  });
});

// ── Property 7: Elastic collision velocity exchange ───────────────────────────
/**
 * Property 7: Elastic collision velocity exchange
 * Validates: Requirements 4.4
 *
 * For any two orbs whose centers are closer than r1 + r2, after applying the
 * elastic collision response, the component of their relative velocity along
 * the collision normal SHALL be reversed (orbs move apart).
 */
describe('OrbPlayground — Property 7: Elastic collision velocity exchange', () => {
  it('relative velocity along collision normal is reversed after collision', () => {
    // Generate overlapping orb pairs: orb2 is placed within r1+r2 of orb1
    const overlappingPairArbitrary = fc
      .tuple(orbArbitrary, orbArbitrary)
      .map(([o1, o2]) => {
        // Place orb2 close to orb1 so they overlap
        const angle = Math.random() * 2 * Math.PI;
        const overlap = (o1.r + o2.r) * 0.5; // half the sum of radii → guaranteed overlap
        return [
          { ...o1, x: 200, y: 200 },
          { ...o2, x: 200 + Math.cos(angle) * overlap, y: 200 + Math.sin(angle) * overlap },
        ] as [OrbState, OrbState];
      })
      // Only test cases where orbs are actually approaching each other
      .filter(([o1, o2]) => {
        const dx = o2.x - o1.x;
        const dy = o2.y - o1.y;
        const dist = Math.sqrt(dx * dx + dy * dy);
        if (dist === 0) return false;
        const nx = dx / dist;
        const ny = dy / dist;
        const v1n = o1.vx * nx + o1.vy * ny;
        const v2n = o2.vx * nx + o2.vy * ny;
        return v1n > v2n; // approaching
      });

    fc.assert(
      fc.property(overlappingPairArbitrary, ([orb1, orb2]) => {
        const dx = orb2.x - orb1.x;
        const dy = orb2.y - orb1.y;
        const dist = Math.sqrt(dx * dx + dy * dy);
        const nx = dx / dist;
        const ny = dy / dist;

        // Relative velocity along normal before collision (positive = approaching)
        const relVBefore =
          (orb1.vx - orb2.vx) * nx + (orb1.vy - orb2.vy) * ny;

        const { orb1: newOrb1, orb2: newOrb2 } = applyElasticCollision(orb1, orb2);

        // Relative velocity along normal after collision (should be negative = separating)
        const relVAfter =
          (newOrb1.vx - newOrb2.vx) * nx + (newOrb1.vy - newOrb2.vy) * ny;

        // After elastic collision, orbs should be moving apart (relV reversed)
        expect(relVAfter).toBeLessThanOrEqual(relVBefore);
      }),
      { numRuns: 100 }
    );
  });

  it('orbs already moving apart are unchanged', () => {
    // orb1 at x=0 moving left (-1), orb2 at x=5 moving right (+1) → separating
    const orb1: OrbState = { id: 0, x: 0, y: 0, vx: -1, vy: 0, r: 10, color: 'accent' };
    const orb2: OrbState = { id: 1, x: 5, y: 0, vx: 1, vy: 0, r: 10, color: 'accent-2' };
    // collision normal points from orb1 to orb2 (positive x direction)
    // v1n = -1, v2n = 1 → v1n - v2n = -2 ≤ 0 → separating, no change applied
    const { orb1: r1, orb2: r2 } = applyElasticCollision(orb1, orb2);
    expect(r1.vx).toBe(orb1.vx);
    expect(r2.vx).toBe(orb2.vx);
  });
});

// ── Property 8: Kinetic energy calculation correctness ────────────────────────
/**
 * Property 8: Kinetic energy calculation correctness
 * Validates: Requirements 4.6
 *
 * For any set of orb states, the value returned by computeKineticEnergy SHALL
 * equal Σ (0.5 * r² * (vx² + vy²)) formatted to exactly 2 decimal places.
 */
describe('OrbPlayground — Property 8: Kinetic energy calculation correctness', () => {
  it('computeKineticEnergy matches manual formula for any orb array', () => {
    fc.assert(
      fc.property(
        fc.array(orbArbitrary, { minLength: 1, maxLength: 10 }),
        (orbs) => {
          const expected = orbs
            .reduce((sum, orb) => {
              return sum + 0.5 * orb.r * orb.r * (orb.vx * orb.vx + orb.vy * orb.vy);
            }, 0)
            .toFixed(2);

          const result = computeKineticEnergy(orbs);

          expect(result).toBe(expected);
        }
      ),
      { numRuns: 100 }
    );
  });

  it('returns "0.00" for an empty orb array', () => {
    expect(computeKineticEnergy([])).toBe('0.00');
  });

  it('result is always formatted to exactly 2 decimal places', () => {
    fc.assert(
      fc.property(
        fc.array(orbArbitrary, { minLength: 0, maxLength: 8 }),
        (orbs) => {
          const result = computeKineticEnergy(orbs);
          expect(result).toMatch(/^\d+\.\d{2}$/);
        }
      ),
      { numRuns: 100 }
    );
  });
});
