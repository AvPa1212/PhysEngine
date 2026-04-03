import React, { useEffect, useRef } from 'react';

export const TECH_STACK = [
  'React 19',
  'TypeScript',
  'Vite',
  'Tailwind CSS',
  'WebAssembly',
  'C++',
  'Three.js',
  'Web Workers',
];

const TECH_COLORS: Record<string, string> = {
  'React 19': 'rgba(97,218,251,0.15)',
  'TypeScript': 'rgba(49,120,198,0.18)',
  'Vite': 'rgba(189,52,254,0.15)',
  'Tailwind CSS': 'rgba(56,189,248,0.15)',
  'WebAssembly': 'rgba(101,79,240,0.18)',
  'C++': 'rgba(0,89,156,0.18)',
  'Three.js': 'rgba(255,255,255,0.08)',
  'Web Workers': 'rgba(109,167,255,0.15)',
};

const TechStackSection: React.FC = () => {
  const badgeRefs = useRef<(HTMLSpanElement | null)[]>([]);

  const reducedMotion =
    typeof window !== 'undefined' &&
    window.matchMedia('(prefers-reduced-motion: reduce)').matches;

  useEffect(() => {
    if (reducedMotion) return;

    badgeRefs.current.forEach((el) => {
      if (!el) return;
      el.style.opacity = '0';
      el.style.transform = 'translateY(12px)';
    });

    let observer: IntersectionObserver | null = null;

    const sectionEl = badgeRefs.current[0]?.parentElement?.parentElement;
    if (!sectionEl) return;

    observer = new IntersectionObserver(
      (entries) => {
        entries.forEach((entry) => {
          if (entry.isIntersecting) {
            badgeRefs.current.forEach((el, i) => {
              if (!el) return;
              el.style.transition = `opacity 350ms ease ${i * 60}ms, transform 350ms ease ${i * 60}ms`;
              el.style.opacity = '1';
              el.style.transform = 'translateY(0)';
            });
            observer?.disconnect();
          }
        });
      },
      { threshold: 0.1 }
    );

    observer.observe(sectionEl);

    return () => {
      observer?.disconnect();
    };
  }, [reducedMotion]);

  return (
    <section aria-label="Tech stack" style={{ paddingBottom: '24px' }}>
      <div
        style={{
          display: 'flex',
          flexWrap: 'wrap',
          gap: '10px',
        }}
      >
        {TECH_STACK.map((tech, i) => (
          <span
            key={tech}
            ref={(el) => { badgeRefs.current[i] = el; }}
            className="task-tag"
            style={
              reducedMotion
                ? {
                    opacity: 1,
                    transform: 'none',
                    background: TECH_COLORS[tech] ?? 'rgba(255,255,255,0.06)',
                    fontFamily: 'var(--mono)',
                    fontSize: '0.8rem',
                    letterSpacing: '0.04em',
                  }
                : {
                    background: TECH_COLORS[tech] ?? 'rgba(255,255,255,0.06)',
                    fontFamily: 'var(--mono)',
                    fontSize: '0.8rem',
                    letterSpacing: '0.04em',
                  }
            }
          >
            {tech}
          </span>
        ))}
      </div>
    </section>
  );
};

export default TechStackSection;
