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

const TechStackSection: React.FC = () => {
  const badgeRefs = useRef<(HTMLSpanElement | null)[]>([]);

  const reducedMotion =
    typeof window !== 'undefined' &&
    window.matchMedia('(prefers-reduced-motion: reduce)').matches;

  useEffect(() => {
    if (reducedMotion) return;

    // Start all badges hidden
    badgeRefs.current.forEach((el) => {
      if (!el) return;
      el.style.opacity = '0';
      el.style.transform = 'translateY(12px)';
    });

    let observer: IntersectionObserver | null = null;

    // Use the section element (parent of badges) as the observation target
    const sectionEl = badgeRefs.current[0]?.parentElement;
    if (!sectionEl) return;

    observer = new IntersectionObserver(
      (entries) => {
        entries.forEach((entry) => {
          if (entry.isIntersecting) {
            badgeRefs.current.forEach((el, i) => {
              if (!el) return;
              el.style.transition = `opacity 300ms ease ${i * 60}ms, transform 300ms ease ${i * 60}ms`;
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
    <section aria-label="Tech stack">
      <div
        style={{
          display: 'flex',
          flexWrap: 'wrap',
          gap: '0.5rem',
        }}
      >
        {TECH_STACK.map((tech, i) => (
          <span
            key={tech}
            ref={(el) => {
              badgeRefs.current[i] = el;
            }}
            className="task-tag"
            style={
              reducedMotion
                ? { opacity: 1, transform: 'none' }
                : undefined
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
