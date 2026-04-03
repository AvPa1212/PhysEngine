// Feature: about-page-interactive, Property 9: Timeline milestone rendering completeness

import * as fc from 'fast-check';
import { render, screen } from '@testing-library/react';
import { vi, beforeEach, describe, it, expect } from 'vitest';
import TimelineSection, { MILESTONES, Milestone } from '../TimelineSection';

// Mock IntersectionObserver — jsdom does not support it
// Mock window.matchMedia — jsdom does not support it
beforeEach(() => {
  global.IntersectionObserver = vi.fn().mockImplementation(() => ({
    observe: vi.fn(),
    unobserve: vi.fn(),
    disconnect: vi.fn(),
  }));

  Object.defineProperty(window, 'matchMedia', {
    writable: true,
    value: vi.fn().mockImplementation((query: string) => ({
      matches: false,
      media: query,
      onchange: null,
      addListener: vi.fn(),
      removeListener: vi.fn(),
      addEventListener: vi.fn(),
      removeEventListener: vi.fn(),
      dispatchEvent: vi.fn(),
    })),
  });
});

/**
 * Property 9: Timeline milestone rendering completeness
 * Validates: Requirements 5.1, 5.4
 *
 * For any array of milestone data with at least 4 entries, the TimelineSection
 * SHALL render each milestone with its date label, title, and description all
 * present in the DOM.
 */
describe('TimelineSection — Property 9: Timeline milestone rendering completeness', () => {
  it('renders all milestone date, title, and description fields for generated arrays (≥4 entries)', () => {
    // Arbitrary for a single milestone with non-empty strings
    const milestoneArbitrary = fc.record<Milestone>({
      date: fc.string({ minLength: 1, maxLength: 20 }).filter((s) => s.trim().length > 0),
      title: fc.string({ minLength: 1, maxLength: 50 }).filter((s) => s.trim().length > 0),
      description: fc.string({ minLength: 1, maxLength: 200 }).filter((s) => s.trim().length > 0),
    });

    fc.assert(
      fc.property(
        fc.array(milestoneArbitrary, { minLength: 4, maxLength: 10 }),
        (milestones) => {
          const { container, unmount } = render(<TimelineSection milestones={milestones} />);
          const text = container.textContent ?? '';

          milestones.forEach((milestone) => {
            // Each field must appear somewhere in the rendered output
            expect(text).toContain(milestone.date);
            expect(text).toContain(milestone.title);
            expect(text).toContain(milestone.description);
          });

          unmount();
        }
      ),
      { numRuns: 100 }
    );
  });

  it('default MILESTONES array has at least 4 entries and all render correctly', () => {
    expect(MILESTONES.length).toBeGreaterThanOrEqual(4);

    render(<TimelineSection />);

    MILESTONES.forEach((milestone) => {
      expect(screen.getByText(milestone.date)).toBeTruthy();
      expect(screen.getByText(milestone.title)).toBeTruthy();
      expect(screen.getByText(milestone.description)).toBeTruthy();
    });
  });

  it('renders a <ul role="list"> with <li role="listitem"> per milestone', () => {
    render(<TimelineSection />);

    const list = screen.getByRole('list');
    expect(list).toBeTruthy();

    const items = screen.getAllByRole('listitem');
    expect(items.length).toBe(MILESTONES.length);
  });

  it('uses IntersectionObserver (not scroll listeners) to trigger animations', () => {
    const observeSpy = vi.fn();
    global.IntersectionObserver = vi.fn().mockImplementation(() => ({
      observe: observeSpy,
      unobserve: vi.fn(),
      disconnect: vi.fn(),
    }));

    render(<TimelineSection />);

    // One IntersectionObserver should be created per milestone
    expect(global.IntersectionObserver).toHaveBeenCalledTimes(MILESTONES.length);
    expect(observeSpy).toHaveBeenCalledTimes(MILESTONES.length);
  });

  it('disconnects all IntersectionObserver instances on unmount', () => {
    const disconnectSpy = vi.fn();
    global.IntersectionObserver = vi.fn().mockImplementation(() => ({
      observe: vi.fn(),
      unobserve: vi.fn(),
      disconnect: disconnectSpy,
    }));

    const { unmount } = render(<TimelineSection />);
    unmount();

    expect(disconnectSpy).toHaveBeenCalledTimes(MILESTONES.length);
  });
});
