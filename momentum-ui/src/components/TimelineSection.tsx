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
  {
    date: 'Q1 2025',
    title: 'Energy & Thermodynamics',
    description:
      'The energy system and thermodynamics engine shipped, adding heat, entropy, and burnout modeling to the simulation.',
  },
  {
    date: 'Q2 2025',
    title: 'Chaos & Quantum Engines',
    description:
      'Butterfly-effect sensitivity analysis and quantum superposition mechanics brought the full five-model physics suite to life.',
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

    itemRefs.current.forEach((el, idx) => {
      if (!el) return;

      el.style.opacity = '0';
      el.style.transform = 'translateY(24px)';
      el.style.transition = `opacity 400ms ease ${idx * 80}ms, transform 400ms ease ${idx * 80}ms`;

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
          paddingLeft: '2rem',
          borderLeft: '2px solid var(--line)',
        }}
      >
        {milestones.map((milestone, i) => (
          <li
            key={`${milestone.date}-${i}`}
            role="listitem"
            ref={(el) => { itemRefs.current[i] = el; }}
            style={
              reducedMotion
                ? { opacity: 1, transform: 'none', marginBottom: '2.5rem', position: 'relative' }
                : { marginBottom: '2.5rem', position: 'relative' }
            }
          >
            {/* Timeline dot */}
            <div aria-hidden="true" style={{
              position: 'absolute',
              left: '-2.6rem',
              top: '0.2rem',
              width: '10px',
              height: '10px',
              borderRadius: '50%',
              background: 'linear-gradient(135deg, var(--accent), var(--accent-2))',
              boxShadow: '0 0 10px rgba(109,167,255,0.5)',
              border: '2px solid var(--bg-1)',
            }} />

            <time
              dateTime={milestone.date}
              style={{
                fontFamily: 'var(--mono)',
                fontSize: '0.72rem',
                letterSpacing: '0.1em',
                opacity: 0.55,
                display: 'block',
                marginBottom: '0.35rem',
                textTransform: 'uppercase',
              }}
            >
              {milestone.date}
            </time>
            <h3
              style={{
                fontFamily: 'var(--mono)',
                fontSize: '1rem',
                margin: '0 0 0.5rem',
                letterSpacing: '0.04em',
              }}
            >
              {milestone.title}
            </h3>
            <p
              style={{
                fontFamily: 'var(--sans)',
                fontSize: '0.875rem',
                opacity: 0.75,
                margin: 0,
                lineHeight: 1.65,
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
