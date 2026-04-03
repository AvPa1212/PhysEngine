import React, { useRef } from 'react';
import ScrollProgressBar from './ScrollProgressBar';
import HeroSection from './HeroSection';
import FeatureCardsSection from './FeatureCardsSection';
import OrbPlayground from './OrbPlayground';
import TimelineSection from './TimelineSection';
import TechStackSection from './TechStackSection';

const AboutPage: React.FC = () => {
  // Cast to HTMLDivElement ref so ScrollProgressBar's prop type is satisfied.
  // The <main> element exposes the same scrollTop/scrollHeight/clientHeight API.
  const scrollContainerRef = useRef<HTMLDivElement>(null);

  return (
    <>
      <ScrollProgressBar scrollContainerRef={scrollContainerRef} />
      <main
        id="about-page"
        ref={scrollContainerRef as unknown as React.RefObject<HTMLElement>}
        style={{
          overflowY: 'auto',
          overscrollBehavior: 'contain',
          minHeight: '100%',
          padding: '0 16px',
        }}
      >
        <HeroSection />
        <FeatureCardsSection />
        <OrbPlayground />
        <TimelineSection />
        <TechStackSection />
      </main>
    </>
  );
};

export default AboutPage;
