// Feature: about-page-interactive, Property 11: Scroll progress calculation correctness

/**
 * Property-based tests for ScrollProgressBar.
 * Validates: Requirements 7.2
 */

import { describe, it, expect } from 'vitest';
import * as fc from 'fast-check';
import { computeScrollProgress } from '../ScrollProgressBar';

describe('computeScrollProgress', () => {
  /**
   * Property 11: Scroll progress calculation correctness
   * Validates: Requirements 7.2
   *
   * For any scroll container state where scrollHeight > clientHeight,
   * the progress value SHALL equal scrollTop / (scrollHeight - clientHeight),
   * clamped to [0, 1].
   */
  it('Property 11: progress equals scrollTop/(scrollHeight-clientHeight) clamped to [0,1]', () => {
    fc.assert(
      fc.property(
        // Generate clientHeight in [1, 10000]
        fc.integer({ min: 1, max: 10_000 }),
        // Generate scrollHeight strictly greater than clientHeight
        fc.integer({ min: 1, max: 10_000 }),
        // Generate scrollTop in [0, scrollHeight] (may exceed range to test clamping)
        fc.integer({ min: 0, max: 20_000 }),
        (clientHeight, extra, scrollTop) => {
          const scrollHeight = clientHeight + extra; // guarantees scrollHeight > clientHeight

          const progress = computeScrollProgress(scrollTop, scrollHeight, clientHeight);

          // Must be clamped to [0, 1]
          expect(progress).toBeGreaterThanOrEqual(0);
          expect(progress).toBeLessThanOrEqual(1);

          // Must equal the formula when scrollTop is within range
          const range = scrollHeight - clientHeight;
          const expected = Math.min(1, Math.max(0, scrollTop / range));
          expect(progress).toBeCloseTo(expected, 10);
        }
      ),
      { numRuns: 100 }
    );
  });

  it('returns 0 when scrollTop is 0', () => {
    expect(computeScrollProgress(0, 1000, 500)).toBe(0);
  });

  it('returns 1 when scrollTop equals scrollHeight - clientHeight', () => {
    expect(computeScrollProgress(500, 1000, 500)).toBe(1);
  });

  it('clamps to 1 when scrollTop exceeds scrollHeight - clientHeight', () => {
    expect(computeScrollProgress(600, 1000, 500)).toBe(1);
  });

  it('returns 0 when scrollHeight equals clientHeight (no scrollable range)', () => {
    expect(computeScrollProgress(0, 500, 500)).toBe(0);
  });

  it('returns 0 when scrollHeight is less than clientHeight', () => {
    expect(computeScrollProgress(0, 400, 500)).toBe(0);
  });
});
