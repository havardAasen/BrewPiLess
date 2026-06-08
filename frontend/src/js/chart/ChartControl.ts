import { BrewChart } from "./BrewChart";
import { ClassLabels, LineIndex } from "./constants";

function bind(id: string, handler: () => void): void {
    const el = document.getElementById(id) as HTMLInputElement;
    if (!el) {
        console.warn(`chart-controls: element #${id} not found`);
        return;
    }
    el.addEventListener("click", handler);
    el.checked = true;
}

export function registerChartControls(bChart: BrewChart): void {
    // Start at second element to match LineIndex
    for (let i = 1; i < ClassLabels.length; i++) {
        const id = ClassLabels[i];
        const line = i as LineIndex;
        bind(id, () => bChart.toggleLine(line));
    }
}
