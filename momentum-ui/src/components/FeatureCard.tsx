import React, { useState } from 'react';

export interface FeatureCardProps {
  title: string;
  icon: string;
  description: string;
  detail: string;
}

const FeatureCard: React.FC<FeatureCardProps> = ({ title, icon, description, detail }) => {
  const [isExpanded, setIsExpanded] = useState(false);

  const toggle = () => setIsExpanded((prev) => !prev);

  const handleKeyDown = (e: React.KeyboardEvent<HTMLDivElement>) => {
    if (e.key === 'Enter' || e.key === ' ') {
      e.preventDefault();
      toggle();
    }
  };

  return (
    <div
      className="panel-card"
      role="button"
      tabIndex={0}
      aria-expanded={isExpanded}
      onClick={toggle}
      onKeyDown={handleKeyDown}
      style={{
        cursor: 'pointer',
        touchAction: 'manipulation',
        minHeight: '44px',
        outline: 'none',
        userSelect: 'none',
      }}
      onFocus={(e) => {
        (e.currentTarget as HTMLDivElement).style.boxShadow =
          '0 0 0 4px rgba(109, 167, 255, 0.44)';
      }}
      onBlur={(e) => {
        (e.currentTarget as HTMLDivElement).style.boxShadow = '';
      }}
    >
      <div style={{ display: 'flex', alignItems: 'center', gap: '10px' }}>
        <span aria-hidden="true" style={{ fontSize: '1.5rem' }}>
          {icon}
        </span>
        <strong style={{ fontFamily: 'var(--mono)', fontSize: '0.95rem' }}>{title}</strong>
      </div>
      <p style={{ margin: '8px 0 0', color: 'var(--muted)', fontSize: '0.88rem' }}>
        {description}
      </p>
      <div
        aria-hidden={!isExpanded}
        style={{
          maxHeight: isExpanded ? '200px' : '0',
          opacity: isExpanded ? 1 : 0,
          overflow: 'hidden',
          transition: 'max-height 220ms ease, opacity 220ms ease',
        }}
      >
        <p style={{ margin: '10px 0 0', fontSize: '0.85rem', lineHeight: 1.55 }}>{detail}</p>
      </div>
    </div>
  );
};

export default FeatureCard;
