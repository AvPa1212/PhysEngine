// Feature: about-page-interactive, Property 10: Tech stack content completeness

import * as fc from 'fast-check';
import { render } from '@testing-library/react';
import { vi, beforeEach, describe, it, expect } from 'vitest';
import TechStackSection, { TECH_STACK } from '../TechStackSection';

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
 * Property 10: Tech stack content completeness
 * Validates: Requirements 6.1
 *
 * For any render of the TechStackSection, all required technologies
 * (React 19, TypeScript, Vite, Tailwind CSS, WebAssembly, C++, Three.js, Web Workers)
 * SHALL be present in the rendered output.
 */
describe('TechStackSection — Property 10: Tech stack content completeness', () => {
  it('renders all 8 required technology names in the output', () => {
    fc.assert(
      fc.property(fc.constant(null), () => {
        const { container, unmount } = render(<TechStackSection />);
        const text = container.textContent ?? '';

        const required = [
          'React 19',
          'TypeScript',
          'Vite',
          'Tailwind CSS',
          'WebAssembly',
          'C++',
          'Three.js',
          'Web Workers',
        ];

        required.forEach((tech) => {
          expect(text).toContain(tech);
        });

        unmount();
      }),
      { numRuns: 100 }
    );
  });

  it('TECH_STACK export contains exactly the 8 required technologies', () => {
    const required = [
      'React 19',
      'TypeScript',
      'Vite',
      'Tailwind CSS',
      'WebAssembly',
      'C++',
      'Three.js',
      'Web Workers',
    ];

    expect(TECH_STACK).toHaveLength(8);
    required.forEach((tech) => {
      expect(TECH_STACK).toContain(tech);
    });
  });

  it('renders badges with task-tag class', () => {
    const { container } = render(<TechStackSection />);
    const badges = container.querySelectorAll('.task-tag');
    expect(badges.length).toBe(TECH_STACK.length);
  });

  it('uses IntersectionObserver to trigger stagger animation', () => {
    const observeSpy = vi.fn();
    global.IntersectionObserver = vi.fn().mockImplementation(() => ({
      observe: observeSpy,
      unobserve: vi.fn(),
      disconnect: vi.fn(),
    }));

    render(<TechStackSection />);

    expect(global.IntersectionObserver).toHaveBeenCalledWith(
      expect.any(Function),
      { threshold: 0.1 }
    );
    expect(observeSpy).toHaveBeenCalledTimes(1);
  });

  it('disconnects IntersectionObserver on unmount', () => {
    const disconnectSpy = vi.fn();
    global.IntersectionObserver = vi.fn().mockImplementation(() => ({
      observe: vi.fn(),
      unobserve: vi.fn(),
      disconnect: disconnectSpy,
    }));

    const { unmount } = render(<TechStackSection />);
    unmount();

    expect(disconnectSpy).toHaveBeenCalled();
  });
});
