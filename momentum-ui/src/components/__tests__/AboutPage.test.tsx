// Feature: about-page-interactive
/**
 * Unit tests for About page routing and navigation.
 * Requirements: 1.1, 1.2, 1.3
 */

import React from 'react';
import { render, screen, cleanup } from '@testing-library/react';
import { MemoryRouter } from 'react-router-dom';
import { vi } from 'vitest';
import AboutPage from '../AboutPage';

// ── Mock usePhysicsWorker so App can render without WASM ──────────────────────
vi.mock('../../hooks/usePhysicsWorker', () => ({
  usePhysicsWorker: () => ({
    isReady: true,
    error: null,
    taskStates: {},
    createTask: vi.fn(),
    destroyTask: vi.fn(),
    applyForce: vi.fn(),
    setMass: vi.fn(),
    collapse: vi.fn(),
    serialize: vi.fn(),
    deserialize: vi.fn(),
    eventBridge: {
      subscribe: vi.fn(() => () => {}),
      clear: vi.fn(),
    },
    perfMonitor: { record: vi.fn() },
  }),
}));

// ── Global mocks needed by all describe blocks that render AboutPage ──────────

function setupGlobalMocks() {
  global.IntersectionObserver = vi.fn().mockImplementation(() => ({
    observe: vi.fn(),
    unobserve: vi.fn(),
    disconnect: vi.fn(),
  }));

  Object.defineProperty(window, 'matchMedia', {
    writable: true,
    value: vi.fn().mockImplementation((query: string) => ({
      matches: false,
      media: query,
      onchange: null,
      addListener: vi.fn(),
      removeListener: vi.fn(),
      addEventListener: vi.fn(),
      removeEventListener: vi.fn(),
      dispatchEvent: vi.fn(),
    })),
  });

  vi.spyOn(window, 'requestAnimationFrame').mockImplementation(() => 1);
  vi.spyOn(window, 'cancelAnimationFrame').mockImplementation(() => {});
}

// ── AboutPage component (standalone) ─────────────────────────────────────────

describe('AboutPage component', () => {
  beforeEach(() => {
    setupGlobalMocks();
  });

  afterEach(() => {
    cleanup();
    vi.restoreAllMocks();
  });

  it('renders a <main> element with id="about-page"', () => {
    render(
      <MemoryRouter>
        <AboutPage />
      </MemoryRouter>
    );
    const main = document.getElementById('about-page');
    expect(main).toBeInTheDocument();
    expect(main?.tagName).toBe('MAIN');
  });

  it('applies overflow-y: auto on the main container', () => {
    render(
      <MemoryRouter>
        <AboutPage />
      </MemoryRouter>
    );
    const main = document.getElementById('about-page') as HTMLElement;
    expect(main.style.overflowY).toBe('auto');
  });

  it('applies overscroll-behavior: contain on the main container', () => {
    render(
      <MemoryRouter>
        <AboutPage />
      </MemoryRouter>
    );
    const main = document.getElementById('about-page') as HTMLElement;
    expect(main.style.overscrollBehavior).toBe('contain');
  });
});

// ── Routing via App (App owns BrowserRouter; navigate via window.history) ─────

async function renderAppAtPath(path: string) {
  window.history.pushState({}, '', path);
  const { default: App } = await import('../../App');
  return render(<App />);
}

describe('/about route', () => {
  beforeEach(() => {
    setupGlobalMocks();
  });

  afterEach(() => {
    cleanup();
    vi.restoreAllMocks();
    window.history.pushState({}, '', '/');
  });

  it('renders the about-page main element when navigating to /about', async () => {
    await renderAppAtPath('/about');

    const main = await screen.findByRole('main', {}, { timeout: 10000 });
    expect(main).toBeInTheDocument();
    expect(main.id).toBe('about-page');
  }, 15000);
});

// ── Navigation bar ────────────────────────────────────────────────────────────

describe('Navigation bar', () => {
  beforeEach(() => {
    setupGlobalMocks();
  });

  afterEach(() => {
    cleanup();
    vi.restoreAllMocks();
    window.history.pushState({}, '', '/');
  });

  it('includes an About tab in the navigation', async () => {
    await renderAppAtPath('/simulation');

    const aboutLinks = screen.getAllByRole('link', { name: /about/i });
    expect(aboutLinks.length).toBeGreaterThanOrEqual(1);
    expect(aboutLinks[0]).toBeInTheDocument();
  });

  it('About tab has the correct href pointing to /about', async () => {
    await renderAppAtPath('/simulation');

    const aboutLink = screen.getByRole('link', { name: /about/i });
    expect(aboutLink).toHaveAttribute('href', '/about');
  });

  it('About tab has active class when on /about route', async () => {
    await renderAppAtPath('/about');

    const aboutLink = screen.getByRole('link', { name: /about/i });
    expect(aboutLink.className).toContain('active');
  });

  it('About tab does not have active class when on /simulation route', async () => {
    await renderAppAtPath('/simulation');

    const aboutLink = screen.getByRole('link', { name: /about/i });
    expect(aboutLink.className).not.toContain('active');
  });
});

// ── Other routes still work ───────────────────────────────────────────────────

describe('Existing routes are unaffected', () => {
  beforeEach(() => {
    setupGlobalMocks();
  });

  afterEach(() => {
    cleanup();
    vi.restoreAllMocks();
    window.history.pushState({}, '', '/');
  });

  it('/simulation route still renders the simulation page', async () => {
    await renderAppAtPath('/simulation');

    expect(screen.getByRole('region', { name: /engine visualization/i })).toBeInTheDocument();
  });

  it('/tasks route still renders the tasks page', async () => {
    await renderAppAtPath('/tasks');

    // Tasks page has a task board heading
    expect(screen.getByText(/task board/i)).toBeInTheDocument();
  });
});

// ── Property 12: Interactive element touch-action ─────────────────────────────
// Feature: about-page-interactive, Property 12: Interactive element touch-action

/**
 * Property 12: Interactive element touch-action
 * Validates: Requirements 8.3
 *
 * Render AboutPage; query all buttons and interactive cards (role="button");
 * verify each has touch-action: manipulation in its inline style.
 */
describe('AboutPage — Property 12: Interactive element touch-action', () => {
  beforeEach(() => {
    // Mock IntersectionObserver
    global.IntersectionObserver = vi.fn().mockImplementation(() => ({
      observe: vi.fn(),
      unobserve: vi.fn(),
      disconnect: vi.fn(),
    }));

    // Mock matchMedia
    Object.defineProperty(window, 'matchMedia', {
      writable: true,
      value: vi.fn().mockImplementation((query: string) => ({
        matches: false,
        media: query,
        onchange: null,
        addListener: vi.fn(),
        removeListener: vi.fn(),
        addEventListener: vi.fn(),
        removeEventListener: vi.fn(),
        dispatchEvent: vi.fn(),
      })),
    });

    // Mock requestAnimationFrame / cancelAnimationFrame
    vi.spyOn(window, 'requestAnimationFrame').mockImplementation((cb) => {
      // Don't actually schedule — return a handle
      return 1 as unknown as number;
    });
    vi.spyOn(window, 'cancelAnimationFrame').mockImplementation(() => {});
  });

  afterEach(() => {
    cleanup();
    vi.restoreAllMocks();
  });

  it('all role="button" elements have touch-action: manipulation', () => {
    render(
      <MemoryRouter>
        <AboutPage />
      </MemoryRouter>
    );

    const buttons = screen.getAllByRole('button');
    expect(buttons.length).toBeGreaterThan(0);

    buttons.forEach((btn) => {
      expect((btn as HTMLElement).style.touchAction).toBe('manipulation');
    });
  });

  it('the OrbPlayground container has touch-action: manipulation', () => {
    render(
      <MemoryRouter>
        <AboutPage />
      </MemoryRouter>
    );

    // The playground container has aria-label="Physics orb playground"
    // Its inner div (the bounded canvas) carries touch-action: manipulation
    const playground = document.querySelector('[aria-label="Physics orb playground"]');
    expect(playground).toBeTruthy();

    // The direct child div is the bounded canvas with touch-action
    const canvas = playground?.querySelector('div') as HTMLElement | null;
    expect(canvas).toBeTruthy();
    expect(canvas!.style.touchAction).toBe('manipulation');
  });
});

// ── Property 15: Animation cleanup on unmount ─────────────────────────────────
// Feature: about-page-interactive, Property 15: Animation cleanup on unmount

/**
 * Property 15: Animation cleanup on unmount
 * Validates: Requirements 1.4
 *
 * Mount then unmount AboutPage; spy on cancelAnimationFrame and
 * IntersectionObserver.disconnect; verify both are called for every
 * registered handle/observer.
 */
describe('AboutPage — Property 15: Animation cleanup on unmount', () => {
  beforeEach(() => {
    // Track all IntersectionObserver disconnect calls
    const disconnectSpy = vi.fn();
    global.IntersectionObserver = vi.fn().mockImplementation(() => ({
      observe: vi.fn(),
      unobserve: vi.fn(),
      disconnect: disconnectSpy,
    }));
    (global.IntersectionObserver as unknown as { _disconnectSpy: typeof disconnectSpy })._disconnectSpy = disconnectSpy;

    Object.defineProperty(window, 'matchMedia', {
      writable: true,
      value: vi.fn().mockImplementation((query: string) => ({
        matches: false,
        media: query,
        onchange: null,
        addListener: vi.fn(),
        removeListener: vi.fn(),
        addEventListener: vi.fn(),
        removeEventListener: vi.fn(),
        dispatchEvent: vi.fn(),
      })),
    });
  });

  afterEach(() => {
    cleanup();
    vi.restoreAllMocks();
  });

  it('cancelAnimationFrame is called on unmount for every RAF handle registered', () => {
    const rafHandles: number[] = [];
    let handleCounter = 0;

    vi.spyOn(window, 'requestAnimationFrame').mockImplementation(() => {
      const handle = ++handleCounter;
      rafHandles.push(handle);
      return handle;
    });

    const cancelSpy = vi.spyOn(window, 'cancelAnimationFrame').mockImplementation(() => {});

    const { unmount } = render(
      <MemoryRouter>
        <AboutPage />
      </MemoryRouter>
    );

    const handlesBeforeUnmount = [...rafHandles];
    unmount();

    // Every handle that was registered should have been cancelled
    handlesBeforeUnmount.forEach((handle) => {
      expect(cancelSpy).toHaveBeenCalledWith(handle);
    });
  });

  it('IntersectionObserver.disconnect is called on unmount for every observer created', () => {
    const disconnectSpy = vi.fn();
    let observerCount = 0;

    global.IntersectionObserver = vi.fn().mockImplementation(() => {
      observerCount++;
      return {
        observe: vi.fn(),
        unobserve: vi.fn(),
        disconnect: disconnectSpy,
      };
    });

    vi.spyOn(window, 'requestAnimationFrame').mockImplementation(() => 1);
    vi.spyOn(window, 'cancelAnimationFrame').mockImplementation(() => {});

    const { unmount } = render(
      <MemoryRouter>
        <AboutPage />
      </MemoryRouter>
    );

    const countBeforeUnmount = observerCount;
    unmount();

    // disconnect should be called once per observer created
    expect(disconnectSpy).toHaveBeenCalledTimes(countBeforeUnmount);
  });
});
