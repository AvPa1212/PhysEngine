import React, { useRef } from 'react';
import ScrollProgressBar from './ScrollProgressBar';
import HeroSection from './HeroSection';
import FeatureCardsSection from './FeatureCardsSection';
import OrbPlayground from './OrbPlayground';
import TimelineSection from './TimelineSection';
import TechStackSection from './TechStackSection';

const AboutPage: React.FC = () => {
  const scrollContainerRef = useRef<HTMLElement>(null);

  return (
    <>
      <ScrollProgressBar scrollContainerRef={scrollContainerRef} />
      <main
        id="about-page"
        ref={scrollContainerRef}
        style={{
          overflowY: 'auto',
          overscrollBehavior: 'contain',
          height: '100%',
          padding: '0',
        }}
      >
        <HeroSection />

        <div style={{ maxWidth: '900px', margin: '0 auto', padding: '0 24px 80px' }}>
          <SectionLabel label="Physics Models" />
          <FeatureCardsSection />

          <SectionLabel label="Interactive Playground" />
          <OrbPlayground />

          <SectionLabel label="Project Timeline" />
          <TimelineSection />

          <SectionLabel label="Built With" />
          <TechStackSection />
        </div>
      </main>
    </>
  );
};

function SectionLabel({ label }: { label: string }) {
  return (
    <div style={{
      display: 'flex',
      alignItems: 'center',
      gap: '12px',
      margin: '64px 0 28px',
    }}>
      <div style={{
        width: '6px',
        height: '6px',
        borderRadius: '50%',
        background: 'linear-gradient(135deg, var(--accent), var(--accent-2))',
        boxShadow: '0 0 12px rgba(109, 167, 255, 0.5)',
        flexShrink: 0,
      }} />
      <span style={{
        fontFamily: 'var(--mono)',
        fontSize: '0.75rem',
        letterSpacing: '0.14em',
        textTransform: 'uppercase',
        color: 'var(--muted)',
      }}>{label}</span>
      <div style={{
        flex: 1,
        height: '1px',
        background: 'linear-gradient(90deg, var(--line-strong), transparent)',
      }} />
    </div>
  );
}

export default AboutPage;
