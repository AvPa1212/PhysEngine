import React, { useEffect, useRef, useState } from 'react';

/**
 * Computes scroll progress as a value clamped to [0, 1].
 * Exported for direct unit/property testing.
 */
export function computeScrollProgress(
  scrollTop: number,
  scrollHeight: number,
  clientHeight: number
): number {
  const range = scrollHeight - clientHeight;
  if (range <= 0) return 0;
  const raw = scrollTop / range;
  return Math.min(1, Math.max(0, raw));
}

interface ScrollProgressBarProps {
  scrollContainerRef: React.RefObject<HTMLElement | HTMLDivElement>;
}

const ScrollProgressBar: React.FC<ScrollProgressBarProps> = ({ scrollContainerRef }) => {
  const [progress, setProgress] = useState(0);
  const reducedMotion = useRef(
    typeof window !== 'undefined' &&
      window.matchMedia('(prefers-reduced-motion: reduce)').matches
  );

  useEffect(() => {
    const el = scrollContainerRef.current;
    if (!el) return;

    const handleScroll = () => {
      setProgress(
        computeScrollProgress(el.scrollTop, el.scrollHeight, el.clientHeight)
      );
    };

    el.addEventListener('scroll', handleScroll, { passive: true });
    return () => {
      el.removeEventListener('scroll', handleScroll);
    };
  }, [scrollContainerRef]);

  return (
    <div
      role="progressbar"
      aria-valuenow={Math.round(progress * 100)}
      aria-valuemin={0}
      aria-valuemax={100}
      style={{
        position: 'fixed',
        top: 0,
        left: 0,
        width: `${progress * 100}%`,
        height: '3px',
        zIndex: 50,
        background: 'linear-gradient(90deg, var(--accent), var(--accent-2), var(--accent-3))',
        transition: reducedMotion.current ? 'none' : 'width 80ms linear',
        pointerEvents: 'none',
      }}
    />
  );
};

export default ScrollProgressBar;
