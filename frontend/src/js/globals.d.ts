declare global {
  interface Window {
    og?: number;
    sg?: number;
    plato?: boolean;
    updateGravity?: (sg: number) => void;
  }
}

export {}; // ensures this file is treated as a module
