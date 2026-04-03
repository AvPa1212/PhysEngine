// Feature: about-page-interactive, Property 2: Feature card count invariant
// Feature: about-page-interactive, Property 3: Feature card content completeness
// Feature: about-page-interactive, Property 4: Feature card expand/collapse round trip
// Feature: about-page-interactive, Property 13: Keyboard operability of Feature Cards

import React from 'react';
import { render, screen, fireEvent, within } from '@testing-library/react';
import * as fc from 'fast-check';
import FeatureCard from '../FeatureCard';
import FeatureCardsSection from '../FeatureCardsSection';

// ── Property 2: Feature card count invariant ──────────────────────────────────
/**
 * Property 2: Feature card count invariant
 * Validates: Requirements 3.1
 *
 * For any render of FeatureCardsSection, the number of FeatureCard children
 * rendered SHALL be exactly 5.
 */
describe('FeatureCardsSection — Property 2: Feature card count invariant', () => {
  it('renders exactly 5 feature cards', () => {
    render(<FeatureCardsSection />);
    // Each FeatureCard has role="button"
    const cards = screen.getAllByRole('button');
    expect(cards).toHaveLength(5);
  });
});

// ── Property 3: Feature card content completeness ─────────────────────────────
/**
 * Property 3: Feature card content completeness
 * Validates: Requirements 3.2
 *
 * For any FeatureCard rendered with valid props, the rendered output SHALL
 * contain the card's title, one-line description, and icon glyph.
 */
describe('FeatureCard — Property 3: Feature card content completeness', () => {
  it('always renders title, description, and icon for any valid props', () => {
    // "valid props" means non-empty, non-whitespace-only strings
    const nonBlankString = (min = 1, max = 80) =>
      fc
        .string({ minLength: min, maxLength: max })
        .filter((s) => s.trim().length > 0);

    fc.assert(
      fc.property(
        fc.record({
          title: nonBlankString(1, 80),
          description: nonBlankString(1, 200),
          icon: nonBlankString(1, 4),
          detail: nonBlankString(1, 400),
        }),
        ({ title, description, icon, detail }) => {
          const { container, unmount } = render(
            <FeatureCard
              title={title}
              description={description}
              icon={icon}
              detail={detail}
            />
          );

          const text = container.textContent ?? '';

          // Title, description, and icon must all appear in the rendered output
          expect(text).toContain(title.trim());
          expect(text).toContain(description.trim());
          expect(text).toContain(icon.trim());

          unmount();
        }
      ),
      { numRuns: 100 }
    );
  });
});

// ── Property 4: Feature card expand/collapse round trip ───────────────────────
/**
 * Property 4: Feature card expand/collapse round trip
 * Validates: Requirements 3.3, 3.4, 9.2
 *
 * For any FeatureCard in its default collapsed state, clicking it once SHALL
 * set aria-expanded to "true" and reveal the detail content; clicking it a
 * second time SHALL set aria-expanded to "false" and hide the detail content,
 * returning the card to its original state.
 */
describe('FeatureCard — Property 4: Feature card expand/collapse round trip', () => {
  const defaultProps = {
    title: 'Test Feature',
    icon: '⚙',
    description: 'A short description.',
    detail: 'Detailed expansion text that should appear on click.',
  };

  it('starts collapsed with aria-expanded="false"', () => {
    render(<FeatureCard {...defaultProps} />);
    const card = screen.getByRole('button');
    expect(card).toHaveAttribute('aria-expanded', 'false');
  });

  it('expands on first click: aria-expanded becomes true', () => {
    render(<FeatureCard {...defaultProps} />);
    const card = screen.getByRole('button');

    fireEvent.click(card);

    expect(card).toHaveAttribute('aria-expanded', 'true');
  });

  it('collapses on second click: aria-expanded returns to false', () => {
    render(<FeatureCard {...defaultProps} />);
    const card = screen.getByRole('button');

    fireEvent.click(card);
    expect(card).toHaveAttribute('aria-expanded', 'true');

    fireEvent.click(card);
    expect(card).toHaveAttribute('aria-expanded', 'false');
  });

  it('detail container is aria-hidden when collapsed', () => {
    render(<FeatureCard {...defaultProps} />);
    // The detail wrapper has aria-hidden="true" when collapsed
    // We check the detail text's parent container
    const card = screen.getByRole('button');
    // Find the detail div (aria-hidden=true when collapsed)
    const detailContainer = card.querySelector('[aria-hidden="true"]');
    expect(detailContainer).not.toBeNull();
  });

  it('detail container is not aria-hidden when expanded', () => {
    render(<FeatureCard {...defaultProps} />);
    const card = screen.getByRole('button');

    fireEvent.click(card);

    // After expansion, aria-hidden should be false on the detail container
    const detailContainer = card.querySelector('[aria-hidden="false"]');
    expect(detailContainer).not.toBeNull();
  });

  it('round-trip: expanded → collapsed returns to original state', () => {
    render(<FeatureCard {...defaultProps} />);
    const card = screen.getByRole('button');

    // Initial state
    expect(card).toHaveAttribute('aria-expanded', 'false');

    // Expand
    fireEvent.click(card);
    expect(card).toHaveAttribute('aria-expanded', 'true');

    // Collapse
    fireEvent.click(card);
    expect(card).toHaveAttribute('aria-expanded', 'false');

    // Verify detail is hidden again
    const detailHidden = card.querySelector('[aria-hidden="true"]');
    expect(detailHidden).not.toBeNull();
  });
});

// ── Property 13: Keyboard operability of Feature Cards ────────────────────────
/**
 * Property 13: Keyboard operability of Feature Cards
 * Validates: Requirements 9.3
 *
 * For any FeatureCard, pressing Enter or Space while the card has keyboard
 * focus SHALL toggle its expanded state, identical to a pointer click.
 */
describe('FeatureCard — Property 13: Keyboard operability of Feature Cards', () => {
  const defaultProps = {
    title: 'Keyboard Test',
    icon: '⚛',
    description: 'Keyboard accessible card.',
    detail: 'This detail is revealed via keyboard.',
  };

  it('toggles expanded state with Enter and Space keys across 100 iterations', () => {
    fc.assert(
      fc.property(
        fc.constantFrom('Enter', ' '),
        (key) => {
          const { unmount } = render(<FeatureCard {...defaultProps} />);
          const card = screen.getByRole('button');

          // Initially collapsed
          expect(card).toHaveAttribute('aria-expanded', 'false');

          // Press key → should expand (same as click)
          fireEvent.keyDown(card, { key });
          expect(card).toHaveAttribute('aria-expanded', 'true');

          // Press key again → should collapse
          fireEvent.keyDown(card, { key });
          expect(card).toHaveAttribute('aria-expanded', 'false');

          unmount();
        }
      ),
      { numRuns: 100 }
    );
  });

  it('Enter key expands the card', () => {
    render(<FeatureCard {...defaultProps} />);
    const card = screen.getByRole('button');

    fireEvent.keyDown(card, { key: 'Enter' });
    expect(card).toHaveAttribute('aria-expanded', 'true');
  });

  it('Space key expands the card', () => {
    render(<FeatureCard {...defaultProps} />);
    const card = screen.getByRole('button');

    fireEvent.keyDown(card, { key: ' ' });
    expect(card).toHaveAttribute('aria-expanded', 'true');
  });

  it('keyboard toggle matches pointer click behavior', () => {
    // Render two identical cards — one toggled by click, one by keyboard
    const { unmount: unmount1 } = render(
      <FeatureCard {...defaultProps} />
    );
    const clickCard = screen.getByRole('button');
    fireEvent.click(clickCard);
    const clickExpanded = clickCard.getAttribute('aria-expanded');
    unmount1();

    const { unmount: unmount2 } = render(
      <FeatureCard {...defaultProps} />
    );
    const keyCard = screen.getByRole('button');
    fireEvent.keyDown(keyCard, { key: 'Enter' });
    const keyExpanded = keyCard.getAttribute('aria-expanded');
    unmount2();

    expect(keyExpanded).toBe(clickExpanded);
  });

  it('other keys do not toggle the card', () => {
    render(<FeatureCard {...defaultProps} />);
    const card = screen.getByRole('button');

    fireEvent.keyDown(card, { key: 'Tab' });
    expect(card).toHaveAttribute('aria-expanded', 'false');

    fireEvent.keyDown(card, { key: 'Escape' });
    expect(card).toHaveAttribute('aria-expanded', 'false');
  });
});
