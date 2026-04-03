import React from 'react';
import FeatureCard from './FeatureCard';

export const FEATURES = [
  {
    id: 'classical',
    title: 'Classical Mechanics',
    icon: '⚙',
    description: 'Newtonian physics drives every task through position, velocity, and force.',
    detail:
      'Tasks behave like physical objects with mass and momentum. Apply forces to accelerate progress, and watch Newton\'s laws govern how work moves through your system. Friction and damping model real-world resistance to completion.',
  },
  {
    id: 'energy',
    title: 'Energy System',
    icon: '⚡',
    description: 'Track kinetic and potential energy across your entire task portfolio.',
    detail:
      'Every task carries an energy budget. Kinetic energy represents active work in progress; potential energy is stored capacity waiting to be released. The system conserves total energy, so overloading one area drains another.',
  },
  {
    id: 'thermo',
    title: 'Thermodynamics',
    icon: '🌡',
    description: 'Heat and entropy model team burnout and workflow disorder.',
    detail:
      'High task density raises system temperature. Entropy increases as tasks pile up unresolved. The thermodynamic engine warns you before your workflow reaches a critical phase transition — giving you time to cool things down.',
  },
  {
    id: 'chaos',
    title: 'Chaos Engine',
    icon: '∿',
    description: 'Butterfly-effect sensitivity reveals hidden dependencies between tasks.',
    detail:
      'Small changes in one task can cascade unpredictably through the system. The chaos engine maps these sensitive dependencies so you can identify which tasks are attractors — stable anchors — and which are chaotic wildcards.',
  },
  {
    id: 'quantum',
    title: 'Quantum Engine',
    icon: '⚛',
    description: 'Superposition and entanglement model uncertainty and task coupling.',
    detail:
      'Tasks exist in superposition until observed — their true state collapses only when you interact with them. Entangled tasks share state: completing one instantly affects its partner, no matter how far apart they are in your backlog.',
  },
] as const;

const FeatureCardsSection: React.FC = () => {
  return (
    <section aria-label="Features">
      <div
        className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3"
        style={{ gap: '14px' }}
      >
        {FEATURES.map((feature) => (
          <FeatureCard
            key={feature.id}
            title={feature.title}
            icon={feature.icon}
            description={feature.description}
            detail={feature.detail}
          />
        ))}
      </div>
    </section>
  );
};

export default FeatureCardsSection;
