import { BrewChart } from "./BrewChart";

export interface BChartType {
    chart: BrewChart | null;
    raw?: unknown;

    toggle(type: string): void;
    init(id: string, y1: string, y2: string): void;
    setIgnoredMask(mask: number): void;
}

export const BChart: BChartType = {
    chart: null,

    toggle(type: string): void {
        this.chart?.toggleLine(type);
    },
    init(id: string, y1: string, y2: string): void {
        this.chart = new BrewChart(id);
        this.chart.setLabels(y1, y2);
    },
    setIgnoredMask(mask: number): void {
        if (!this.chart) return;
        if (this.chart.cal_igmask === mask) return;

        this.chart.calculateSG = false;
        this.chart.process(this.raw);
        // the data will be updated by the "data"
        this.chart.cal_igmask = mask;
        this.chart.getFormula();

        this.chart.process(this.raw);

        this.chart.updateChart();
        // the data will be updated by the "data",again
        this.chart.cal_igmask = mask;
    },
};
