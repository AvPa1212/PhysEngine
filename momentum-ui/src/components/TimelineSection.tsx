import React, { useEffect, useRef } from 'react';

export interface Milestone {
  date: string;
  title: string;
  description: string;
}

export const MILESTONES: Milestone[] = [
  {
    date: 'Q1 2024',
    title: 'Project Inception',
    description:
      'Momentum was conceived as a physics-based task manager, combining Newtonian mechanics with productivity workflows.',
  },
  {
    date: 'Q2 2024',
    title: 'Classical Engine',
    description:
      'The classical mechanics engine was implemented in C++, enabling tasks to carry mass, velocity, and momentum properties.',
  },
  {
    date: 'Q3 2024',
    title: 'WebAssembly Port',
    description:
      'The C++ physics core was compiled to WebAssembly via Emscripten, bringing native-speed simulation to the browser.',
  },
  {
    date: 'Q4 2024',
    title: 'React UI Launch',
    description:
      'The React/TypeScript frontend launched with the dark glassmorphism aesthetic, aurora gradients, and real-time physics visualizations.',
  },
];

interface TimelineSectionProps {
  milestones?: Milestone[];
}

const TimelineSection: React.FC<TimelineSectionProps> = ({
  milestones = MILESTONES,
}) => {
  const itemRefs = useRef<(HTMLLIElement | null)[]>([]);

  const reducedMotion =
    typeof window !== 'undefined' &&
    window.matchMedia('(prefers-reduced-motion: reduce)').matches;

  useEffect(() => {
    if (reducedMotion) return;

    const observers: IntersectionObserver[] = [];

    itemRefs.current.forEach((el) => {
      if (!el) return;

      // Start hidden
      el.style.opacity = '0';
      el.style.transform = 'translateY(24px)';
      el.style.transition = 'opacity 400ms ease, transform 400ms ease';

      const observer = new IntersectionObserver(
        (entries) => {
          entries.forEach((entry) => {
            if (entry.isIntersecting) {
              (entry.target as HTMLElement).style.opacity = '1';
              (entry.target as HTMLElement).style.transform = 'translateY(0)';
              observer.unobserve(entry.target);
            }
          });
        },
        { threshold: 0.2 }
      );

      observer.observe(el);
      observers.push(observer);
    });

    return () => {
      observers.forEach((obs) => obs.disconnect());
    };
  }, [reducedMotion, milestones]);

  return (
    <section aria-label="Project timeline">
      <ul
        role="list"
        style={{
          listStyle: 'none',
          margin: 0,
          padding: 0,
          paddingLeft: '1.5rem',
          borderLeft: '2px solid var(--line)',
        }}
      >
        {milestones.map((milestone, i) => (
          <li
            key={`${milestone.date}-${i}`}
            role="listitem"
            ref={(el) => {
              itemRefs.current[i] = el;
            }}
            style={
              reducedMotion
                ? { opacity: 1, transform: 'none', marginBottom: '2rem' }
                : { marginBottom: '2rem' }
            }
          >
            <time
              dateTime={milestone.date}
              style={{
                fontFamily: 'var(--mono)',
                fontSize: '0.75rem',
                opacity: 0.6,
                display: 'block',
                marginBottom: '0.25rem',
              }}
            >
              {milestone.date}
            </time>
            <h3
              style={{
                fontFamily: 'var(--mono)',
                fontSize: '1rem',
                margin: '0 0 0.5rem',
              }}
            >
              {milestone.title}
            </h3>
            <p
              style={{
                fontFamily: 'var(--sans)',
                fontSize: '0.875rem',
                opacity: 0.8,
                margin: 0,
              }}
            >
              {milestone.description}
            </p>
          </li>
        ))}
      </ul>
    </section>
  );
};

export default TimelineSection;
