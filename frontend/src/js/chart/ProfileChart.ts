import Dygraph from "dygraphs";
import { byId } from "../shared";

type TempUnit = "C" | "F";
type ChartPoint = [Date | number];

export class ProfileChart {
    private unit: TempUnit = "C";
    private readonly chart: Dygraph;

    constructor(divId: string, data: ChartPoint[], unit: TempUnit) {
        const container = byId(divId);
        if (!container) {
            throw new Error(`ControlChart.init: Element #${divId} not found`);
        }

        this.unit = unit;

        this.chart = new Dygraph(container, data, {
            colors: ["rgb(89, 184, 255)"],
            axisLabelFontSize: 12,
            gridLineColor: "#ccc",
            gridLineWidth: 0.1,
            labels: [
                "<%= script_control_time %>",
                "<%= script_control_temperature %>",
            ],
            legend: "always",
            labelsDivStyles: {
                textAlign: "right",
            },
            strokeWidth: 1,

            axes: {
                y: {
                    valueFormatter: this.temperatureFormatter,
                    pixelsPerLabel: 20,
                    axisLabelWidth: 35,
                },
                x: {
                    axisLabelFormatter: this.shortDateFormatter,
                    valueFormatter: this.dateFormatter,
                    pixelsPerLabel: 30,
                    axisLabelWidth: 40,
                },
            },

            highlightCircleSize: 2,
            highlightSeriesOpts: {
                strokeWidth: 1.5,
                strokeBorderWidth: 1,
                highlightCircleSize: 5,
            },
        });
    }

    update(data: ChartPoint[], unit: TempUnit): void {
        if (data.length == 0) return;

        this.unit = unit;

        this.chart.updateOptions({ file: data });
    }

    private dateFormatter(v: number): string {
        const d = new Date(v);
        return d.shortLocalizedString();
    }

    private shortDateFormatter(v: number | Date): string {
        const d = new Date(v);
        const y = d.getFullYear();
        const re = new RegExp(`[^\d]?${y}[^\d]?`);
        return d.toLocaleDateString().replace(re, "");
    }

    private temperatureFormatter = (v: number): string =>
        `${v.toFixed(1)}&deg;${this.unit}`;
}
