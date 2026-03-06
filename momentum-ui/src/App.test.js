import { render, screen } from '@testing-library/react';
import App from './App';

test('shows the quantum core loading screen while the WASM engine initialises', () => {
  render(<App />);
  // The engine script is injected asynchronously; before it resolves the app
  // renders the loading indicator.
  const loader = screen.getByText(/loading quantum core/i);
  expect(loader).toBeInTheDocument();
});
