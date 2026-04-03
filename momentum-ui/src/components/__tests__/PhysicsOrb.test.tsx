// Feature: about-page-interactive, Property 14: Orb aria-hidden invariant
import React from 'react';
import { render } from '@testing-library/react';
import * as fc from 'fast-check';
import PhysicsOrb from '../PhysicsOrb';

const COLORS = ['accent', 'accent-2', 'accent-3'] as const;

/**
 * Property 14: Orb aria-hidden invariant
 * Validates: Requirements 9.4
 *
 * For any number of PhysicsOrb components rendered (1–8),
 * every SVG element SHALL have aria-hidden="true".
 */
describe('PhysicsOrb', () => {
  it('Property 14: every rendered SVG has aria-hidden="true" for any orb count (1–8)', () => {
    fc.assert(
      fc.property(
        fc.integer({ min: 1, max: 8 }),
        (orbCount) => {
          const orbs = Array.from({ length: orbCount }, (_, i) => (
            <PhysicsOrb
              key={i}
              size={20 + i * 5}
              color={COLORS[i % COLORS.length]}
              offsetX={i * 10}
              offsetY={i * 5}
            />
          ));

          const { container } = render(<div>{orbs}</div>);
          const svgElements = container.querySelectorAll('svg');

          expect(svgElements.length).toBe(orbCount);

          svgElements.forEach((svg) => {
            expect(svg.getAttribute('aria-hidden')).toBe('true');
          });
        }
      ),
      { numRuns: 100 }
    );
  });
});
