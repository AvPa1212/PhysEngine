// Feature: about-page-interactive, Property 1: Pointer offset clamping
import * as fc from 'fast-check';
import { computeOrbOffset } from '../HeroSection';

/**
 * Property 1: Pointer offset clamping
 * Validates: Requirements 2.5
 *
 * For any pointer position and any Physics_Orb center, when the pointer is
 * within 120 px of the orb, the computed CSS translate offset applied to the
 * orb SHALL be at most 24 px in each axis.
 */
describe('HeroSection — computeOrbOffset', () => {
  it('Property 1: offset is clamped to ±24 px on each axis for any pointer/orb position', () => {
    fc.assert(
      fc.property(
        fc.record({
          pointerX: fc.float({ min: -10000, max: 10000, noNaN: true }),
          pointerY: fc.float({ min: -10000, max: 10000, noNaN: true }),
          orbCenterX: fc.float({ min: -10000, max: 10000, noNaN: true }),
          orbCenterY: fc.float({ min: -10000, max: 10000, noNaN: true }),
        }),
        ({ pointerX, pointerY, orbCenterX, orbCenterY }) => {
          const { offsetX, offsetY } = computeOrbOffset(
            pointerX,
            pointerY,
            orbCenterX,
            orbCenterY
          );

          expect(Math.abs(offsetX)).toBeLessThanOrEqual(24);
          expect(Math.abs(offsetY)).toBeLessThanOrEqual(24);
        }
      ),
      { numRuns: 100 }
    );
  });

  it('returns zero offset when pointer is outside 120 px radius', () => {
    fc.assert(
      fc.property(
        fc.record({
          // Pointer far away: at least 121 px from orb center
          pointerX: fc.float({ min: 200, max: 10000, noNaN: true }),
          pointerY: fc.float({ min: 0, max: 0, noNaN: true }),
          orbCenterX: fc.constant(0),
          orbCenterY: fc.constant(0),
        }),
        ({ pointerX, pointerY, orbCenterX, orbCenterY }) => {
          const { offsetX, offsetY } = computeOrbOffset(
            pointerX,
            pointerY,
            orbCenterX,
            orbCenterY
          );

          expect(offsetX).toBe(0);
          expect(offsetY).toBe(0);
        }
      ),
      { numRuns: 100 }
    );
  });
});
