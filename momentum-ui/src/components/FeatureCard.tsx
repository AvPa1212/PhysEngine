import React, { useState } from 'react';

export interface FeatureCardProps {
  title: string;
  icon: string;
  description: string;
  detail: string;
}

const FeatureCard: React.FC<FeatureCardProps> = ({ title, icon, description, detail }) => {
  const [isExpanded, setIsExpanded] = useState(false);
  const [isHovered, setIsHovered] = useState(false);

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
      onMouseEnter={() => setIsHovered(true)}
      onMouseLeave={() => setIsHovered(false)}
      onFocus={(e) => {
        (e.currentTarget as HTMLDivElement).style.boxShadow =
          '0 0 0 4px rgba(109, 167, 255, 0.44)';
      }}
      onBlur={(e) => {
        (e.currentTarget as HTMLDivElement).style.boxShadow = '';
      }}
      style={{
        cursor: 'pointer',
        touchAction: 'manipulation',
        minHeight: '44px',
        outline: 'none',
        userSelect: 'none',
        transform: isHovered ? 'translateY(-4px)' : 'translateY(0)',
        transition: 'transform 0.22s ease, box-shadow 0.22s ease, border-color 0.22s ease',
        borderColor: isHovered ? 'rgba(109, 167, 255, 0.28)' : undefined,
        boxShadow: isHovered ? '0 20px 48px rgba(0,0,0,0.28)' : undefined,
      }}
    >
      <div style={{ display: 'flex', alignItems: 'center', gap: '12px' }}>
        <span
          aria-hidden="true"
          style={{
            fontSize: '1.6rem',
            lineHeight: 1,
            filter: isHovered ? 'drop-shadow(0 0 8px rgba(109,167,255,0.5))' : 'none',
            transition: 'filter 0.22s ease',
          }}
        >
          {icon}
        </span>
        <strong style={{ fontFamily: 'var(--mono)', fontSize: '0.92rem', letterSpacing: '0.04em' }}>
          {title}
        </strong>
        <span style={{
          marginLeft: 'auto',
          fontFamily: 'var(--mono)',
          fontSize: '0.7rem',
          color: 'var(--muted)',
          opacity: 0.6,
          transition: 'transform 0.22s ease',
          transform: isExpanded ? 'rotate(180deg)' : 'rotate(0deg)',
          display: 'inline-block',
        }}>▾</span>
      </div>
      <p style={{ margin: '10px 0 0', color: 'var(--muted)', fontSize: '0.875rem', lineHeight: 1.55 }}>
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
        <div style={{
          marginTop: '12px',
          paddingTop: '12px',
          borderTop: '1px solid var(--line)',
        }}>
          <p style={{ margin: 0, fontSize: '0.85rem', lineHeight: 1.65, color: 'var(--text)', opacity: 0.85 }}>
            {detail}
          </p>
        </div>
      </div>
    </div>
  );
};

export default FeatureCard;
