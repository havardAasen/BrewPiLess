declare global {
    interface Window {
        og?: number;
        sg?: number;
        plato?: boolean;
        updateGravity?: (sg: number) => void;
        oridata: Record<string, string | number | boolean>;
        Net: typeof Net;
        saveSystemSettings: () => void;
    }
}

export {}; // ensures this file is treated as a module
