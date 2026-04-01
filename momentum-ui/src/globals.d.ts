declare global {
  interface Window {
    PhysEngine: (opts: {
      locateFile: (path: string) => string;
    }) => Promise<any>;
  }
}

export {};
