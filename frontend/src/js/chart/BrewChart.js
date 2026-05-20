import regression from "regression";
import { BrewMath, byId, C2F, select } from "../shared";
import { ClassLabels, Colors, Labels, LineIndex, ModeMap } from "./constants";
import { gravityFilter } from "./GravityFilter";
import { gravityTracker } from "./GravityTracker";
import { STATES, STATE_LINE_WIDTH } from "./common";
import Dygraph from "dygraphs";

export class BrewChart {
    // time & config
    ctime = 0;
    starttime = 0;
    interval = 60;
    celsius = true;
    plato = false;
    calibrating = false;

    // data store
    data = [];
    anno = [];
    state = [];
    /** @type number[] */
    angles = [];
    /** @type number[] */
    rawSG = [];

    // working state
    laststat = [];
    lidx = 0;
    numData = 8;
    changes = 0;
    dataset = [];
    cstate = 0;
    coTemp = 20;
    cal_igmask = 0;

    // calibration / sg
    calculateSG = false;
    /** @type {(x: number) => number} */
    sgByTilt = () => NaN;
    calibrationPoints = [];
    tiltInWater = 0;
    readingInWater = 0;
    og = NaN;
    filterSg = NaN;
    sg = NaN;

    // UI
    ylabel;
    y2label;
    chart;
    cid;

    constructor(cid) {
        this.cid = cid;
        this.clearData();
        gravityFilter.reset();
        gravityTracker.init();
    }

    clearData() {
        this.laststat = [NaN, NaN, NaN, NaN, NaN, NaN, NaN, NaN];
        this.sg = NaN;
        this.og = NaN;
    }

    getModeBeforeTime(start) {
        if (!this.anno || this.anno.length === 0) return "p".charCodeAt(0);
        let mode = this.anno[0].shortText;
        const allmode = "OBPF";
        for (let i = 0; i < this.anno.length; i++) {
            if (this.anno[i].x > start) break;
            if (allmode.indexOf(this.anno[i].shortText) >= 0)
                mode = this.anno[i].shortText;
        }
        return mode.charCodeAt(0);
    }

    getTiltAround(idx) {
        if (!isNaN(this.angles[idx]))
            return [this.angles[idx], this.data[idx][LineIndex.AuxTemp]];
        let left = -1,
            right = -1;
        for (let i = idx - 1; i >= 0; i--)
            if (!isNaN(this.angles[idx])) {
                left = i;
                break;
            }
        for (let i = idx + 1; i < this.angles.length; i++)
            if (!isNaN(this.angles[idx])) {
                right = i;
                break;
            }
        if (left < 0 && right < 0) return null;
        if (left < 0)
            return [this.angles[right], this.data[right][LineIndex.AuxTemp]];
        if (right < 0)
            return [this.angles[left], this.data[left][LineIndex.AuxTemp]];
        const tilt =
            this.angles[left] +
            ((this.angles[right] - this.angles[left]) / (right - left)) *
                (idx - left);
        const aux =
            (this.data[left][LineIndex.AuxTemp] +
                this.data[right][LineIndex.AuxTemp]) /
            2;
        return [tilt, aux];
    }

    getCalibration() {
        const pairs = [];
        for (let i = 0; i < this.data.length; i++) {
            const g = this.data[i][LineIndex.Gravity];
            if (g) {
                const pair = this.getTiltAround(i);
                if (pair) {
                    const beerTemp = this.celsius ? C2F(pair[1]) : pair[1];
                    const gravity = g;
                    let converted;
                    if (this.plato)
                        converted = BrewMath.sg2pla(
                            BrewMath.tempCorrectionF(
                                BrewMath.pla2sg(gravity),
                                C2F(this.coTemp),
                                beerTemp,
                            ),
                        );
                    else
                        converted = BrewMath.tempCorrectionF(
                            gravity,
                            C2F(this.coTemp),
                            beerTemp,
                        );
                    pairs.push([pair[0], converted]);
                }
            }
        }
        pairs.push([this.tiltInWater, this.readingInWater]);
        return pairs;
    }

    filterPoints(points, mask) {
        const out = [];
        for (let i = 0; i < points.length; i++)
            if (!(mask & (0x1 << i))) out.push(points[i]);
        return out;
    }

    setIgnoredMask(mask) {
        if (this.cal_igmask === mask) return false;
        this.cal_igmask = mask;
        return true;
    }

    getFormula() {
        const points = this.getCalibration();
        if (points.length < 2) return;
        let cpoints = this.filterPoints(points, this.cal_igmask);
        if (cpoints.length < 2) {
            cpoints = points;
            this.cal_igmask = 0;
        }
        const poly = regression(
            "polynomial",
            cpoints,
            cpoints.length > 3 ? 3 : cpoints.length > 2 ? 2 : 1,
            { precision: 9 },
        );
        this.calibrationPoints = points;
        this.calculateSG = true;
        if (cpoints.length > 3) {
            this.sgByTilt = (x) =>
                poly.equation[0] +
                poly.equation[1] * x +
                poly.equation[2] * x * x +
                poly.equation[3] * x * x * x;
        } else if (cpoints.length > 2) {
            this.sgByTilt = (x) =>
                poly.equation[0] +
                poly.equation[1] * x +
                poly.equation[2] * x * x;
        } else {
            this.sgByTilt = (x) => poly.equation[0] + poly.equation[1] * x;
        }
        this.coefficients =
            cpoints.length > 3
                ? [
                      poly.equation[0],
                      poly.equation[1],
                      poly.equation[2],
                      poly.equation[3],
                  ]
                : cpoints.length > 2
                  ? [poly.equation[0], poly.equation[1], poly.equation[2], 0]
                  : [poly.equation[0], poly.equation[1], 0, 0];
        this.npt = points.length;
    }

    process(bytes) {
        let i = 0;
        let newchart = false;
        let sgPoint = false;
        this.filterSg = NaN;

        const data = Array.from(bytes);

        while (i < data.length) {
            const d0 = data[i++];
            const d1 = data[i++];
            if (d0 === 0xff) {
                if ((d1 & 0xf) !== 5) throw new Error("version mismatched");
                this.celsius = d1 & 0x10 ? false : true;
                this.calibrating = d1 & 0x20 ? false : true;
                this.plato = d1 & 0x40 ? false : true;

                let p = data[i++];
                p = p * 256 + data[i++];
                this.interval = p;
                this.starttime =
                    (data[i] << 24) +
                    (data[i + 1] << 16) +
                    (data[i + 2] << 8) +
                    data[i + 3];
                this.ctime = this.starttime;
                i += 4;
                this.data = [];
                this.anno = [];
                this.state = [];
                this.angles = [];
                this.rawSG = [];
                this.cstate = 0;
                this.coTemp = 20;
                this.cal_igmask = 0;
                this.clearData();
                newchart = true;
                gravityFilter.reset();
                gravityTracker.init();
            } else if (d0 === 0xf3) {
                this.coTemp = d1;
            } else if (d0 === 0xf4) {
                this.addMode(d1, this.ctime * 1000);
            } else if (d0 === 0xf1) {
                this.cstate = d1;
            } else if (d0 === 0xfe) {
                this.lidx = 0;
                const d2 = data[i++],
                    d3 = data[i++];
                let tdiff = d3 + (d2 << 8) + (d1 << 16);
                if (tdiff > 30 * 24 * 60 * 60) tdiff = 30 * 60;
                const ntime = this.starttime + tdiff;
                if (ntime > this.ctime) {
                    this.data.push([
                        new Date(this.ctime * 1000),
                        NaN,
                        NaN,
                        NaN,
                        NaN,
                        NaN,
                        NaN,
                        NaN,
                        NaN,
                    ]);
                    this.state.push(null);
                    this.angles.push(NaN);
                    this.rawSG.push(NaN);
                    this.ctime =
                        ntime - this.ctime > this.interval
                            ? ntime
                            : this.ctime + this.interval;
                }
                this.anno.push({
                    series: "beerTemp",
                    x: this.ctime * 1000,
                    shortText: "R",
                    text: "Resume",
                    attachAtBottom: true,
                });
            } else if (d0 === 0xf8) {
                const hh = data[i++],
                    ll = data[i++];
                const v = (hh & 0x7f) * 256 + ll;
                this.og = this.plato ? v / 100 : v / 10000;
            } else if (d0 === 0xfa) {
                const b2 = data[i++],
                    b3 = data[i++];
                this.cal_igmask = (d1 << 14) + (b2 << 7) + b3;
            } else if (d0 === 0xf9) {
                const hh = data[i++],
                    ll = data[i++];
                const v = (hh & 0x7f) * 256 + ll;
                this.tiltInWater = v / 100;
                this.readingInWater = this.plato
                    ? 0
                    : d1 === 0
                      ? 1.0
                      : 0.9 + d1 / 1000;
            } else if (d0 === 0xf0) {
                this.changes = d1;
                this.lidx = 0;
                this.dataset = [new Date(this.ctime * 1000)];
                this.processRecord();
            } else if (d0 < 128) {
                let tp = d0 * 256 + d1;
                if (this.lidx === LineIndex.Gravity) {
                    tp =
                        tp === 0x7fff
                            ? NaN
                            : this.plato
                              ? tp / 100
                              : tp > 8000
                                ? tp / 10000
                                : tp / 1000;
                    sgPoint = true;
                } else if (this.lidx === LineIndex.Tilt) {
                    tp = tp === 0x7fff ? NaN : tp / 100;
                } else {
                    tp = tp === 0x7fff ? NaN : tp / 100;
                    if (tp >= 225) tp = 225 - tp;
                }
                if (this.lidx < this.numData) {
                    this.dataset.push(tp);
                    this.laststat[this.lidx] = tp;
                    this.lidx++;
                    this.processRecord();
                }
            }
        }

        return { nc: newchart, sg: sgPoint };
    }

    processRecord() {
        while (
            ((1 << this.lidx) & this.changes) === 0 &&
            this.lidx < this.numData
        ) {
            this.dataset.push(
                this.lidx > LineIndex.RoomTemp
                    ? null
                    : this.laststat[this.lidx],
            );
            this.lidx++;
        }
        if (this.lidx >= this.numData) {
            const dataset = this.dataset.slice(0, 8);
            const rawSG = dataset[LineIndex.Gravity];
            let sg = NaN;
            if (!this.calculateSG && dataset[LineIndex.Gravity] != null)
                sg = dataset[LineIndex.Gravity];
            else if (this.calculateSG) {
                if (this.dataset[8] == null) dataset[LineIndex.Gravity] = null;
                else {
                    const temp = this.celsius
                        ? C2F(dataset[LineIndex.AuxTemp])
                        : dataset[LineIndex.AuxTemp];
                    sg = this.sgByTilt(this.dataset[8]);
                    if (this.plato)
                        sg = BrewMath.sg2pla(
                            BrewMath.tempCorrectionF(
                                BrewMath.pla2sg(sg),
                                temp,
                                C2F(this.coTemp),
                            ),
                        );
                    dataset[LineIndex.Gravity] = sg;
                }
            }
            if (!isNaN(sg)) {
                this.sg = sg;
                this.filterSg = gravityFilter.add(sg);
                if (this.plato)
                    gravityTracker.add(
                        Math.round(this.filterSg * 10),
                        this.ctime,
                    );
                else
                    gravityTracker.add(
                        Math.round(this.filterSg * 1000),
                        this.ctime,
                    );
            }
            if (!isNaN(this.sg)) dataset.push(this.filterSg);
            else dataset.push(null);

            this.data.push(dataset);
            this.state.push(this.cstate);
            this.angles.push(this.dataset[8] ?? NaN);
            this.rawSG.push(rawSG ?? NaN);
            this.ctime += this.interval;
        }
    }

    incTime() {
        // format time, use hour and minute only.
        this.ctime += this.interval;
        //	console.log("incTime:"+ this.ctime/this.interval);
    }

    formatDate(d) {
        var HH = d.getHours();
        var MM = d.getMinutes();
        var SS = d.getSeconds();

        function T(x) {
            return x > 9 ? x : "0" + x;
        }
        return d.toLocaleDateString() + " " + T(HH) + ":" + T(MM) + ":" + T(SS);
    }

    /**
     * @param {number} elapsed Seconds
     * @returns {string}
     */
    formatDuration(elapsed) {
        const days = Math.floor(elapsed / 86400);
        const hours = (elapsed % 86400) / 3600;

        let result = "";
        if (days > 0) result += days + "d";
        result += hours.toFixed(1) + "h";

        return result;
    }

    /**
     * @param {Date} date
     * @param {number} row
     */
    showLegend(date, row) {
        const d = new Date(date);
        select(".beer-chart-legend-time").innerHTML = this.formatDate(d);

        const elapseEl = select(".beer-chart-legend-elapse");
        if (elapseEl)
            elapseEl.innerHTML = this.formatDuration(
                d.getTime() / 1000 - this.starttime,
            );

        ClassLabels.forEach((label, i) => {
            if (i === 0) return;

            const value = this.chart.getValue(row, i);
            const el = select(`.chart-legend-row.${label} .legend-value`);
            if (!el) return;

            if (label === "gravity" || label === "filtersg") {
                el.innerHTML = Number.isFinite(value)
                    ? this.plato
                        ? `${value.toFixed(2)}&deg;P`
                        : value.toFixed(4)
                    : "--";
            } else {
                el.innerHTML = this.tempFormat(value);
            }
        });

        const state = parseInt(this.state[row]);
        if (!isNaN(state)) {
            select(".beer-chart-state").innerHTML = STATES[state].text;
        }
    }

    hideLegend() {
        var v = document.querySelectorAll(".legend-value");
        v.forEach(function (val) {
            val.innerHTML = "--";
        });
        select(".beer-chart-legend-time").innerHTML = this.dateLabel; //"Date/Time";
        select(".beer-chart-state").innerHTML = "<%= chart_state_label %>";
    }

    /**
     * @param {string | number} temperature
     * @returns {string}
     */
    tempFormat(temperature) {
        const value = Number(temperature);
        if (!Number.isFinite(value)) return "--";

        const unit = this.celsius ? "&deg;C" : "&deg;F";
        return `${value.toFixed(2)}${unit}`;
    }

    initLegend() {
        const toggle = select(".beer-temp .toggle");
        if (!toggle) return;

        for (var i = 1; i < ClassLabels.length; i++) {
            const label = ClassLabels[i];
            const color = Colors[i - 1];

            select(`.chart-legend-row.${label}`).style.color = color;
            select(`.${label}.toggle`).style.backgroundColor = color;
        }

        this.dateLabel = select(".beer-chart-legend-time").innerHTML;
    }

    /** @param {number} index */
    toggleLine(index) {
        const visibleSeries = this.chart.visibility();
        this.chart.setVisibility(index - 1, !visibleSeries[index - 1]);
    }

    setLabels(y1, y2) {
        this.ylabel = y1;
        this.y2label = y2;
    }

    createChart() {
        var t = this;
        t.initLegend();

        var ldiv = document.createElement("div");
        ldiv.className = "hide";
        var ylabel =
            (t.ylabel ? t.ylabel : "Temperature") +
            "(&deg;" +
            (t.celsius ? "C" : "F") +
            ")";
        var y2label = t.y2label ? t.y2label : "Gravity";
        document.body.appendChild(ldiv);
        var opt = {
            labels: Labels,
            colors: Colors,
            connectSeparatedPoints: true,
            ylabel: ylabel,
            y2label: y2label,
            series: {
                gravity: {
                    axis: "y2",
                    drawPoints: true,
                    pointSize: 2,
                    highlightCircleSize: 4,
                },
                filtersg: {
                    axis: "y2",
                },
            },
            axisLabelFontSize: 12,
            animatedZooms: true,
            gridLineColor: "#ccc",
            gridLineWidth: "0.1px",
            labelsDiv: ldiv,
            labelsDivStyles: {
                display: "none",
            },
            displayAnnotations: true,
            //showRangeSelector: true,
            strokeWidth: 1,
            axes: {
                y: {
                    valueFormatter: function (y) {
                        return t.tempFormat(y);
                    },
                },
                y2: {
                    valueFormatter: function (y) {
                        return t.plato ? y.toFixed(1) : y.toFixed(3);
                    },
                    axisLabelFormatter: function (y) {
                        var range = this.yAxisRange(1);
                        if (t.plato)
                            return range[1] - range[0] > 1
                                ? y.toFixed(1)
                                : y.toFixed(2);

                        if (range[1] - range[0] > 0.002)
                            return y.toFixed(3).substring(1);
                        else return y.toFixed(4).substring(2);
                    },
                },
            },
            highlightCircleSize: 2,
            highlightSeriesOpts: {
                strokeWidth: 1.5,
                strokeBorderWidth: 1,
                highlightCircleSize: 5,
            },
            highlightCallback: function (e, x, pts, row) {
                t.showLegend(x, row);
            },
            unhighlightCallback: function () {
                t.hideLegend();
            },
            underlayCallback: function (ctx, area, graph) {
                ctx.save();
                try {
                    t.drawBackground(ctx, area, graph);
                } finally {
                    ctx.restore();
                }
            },
            /*                drawCallback: function(beerChart, is_initial) {
									if (is_initial) {
										if (t.anno.length > 0) {
											t.chart.setAnnotations(t.anno);
										}
									}
								}*/
        };
        t.chart = new Dygraph(byId(t.cid), t.data, opt);
    }

    updateChart() {
        var t = this;
        if (typeof t.chart == "undefined") t.createChart();
        else
            t.chart.updateOptions({
                file: t.data,
            });
        t.chart.setAnnotations(t.anno);
    }

    /**
     * @param {Dygraph} g
     * @param {number} time
     * @returns {number}
     */
    findNearestRow(g, time) {
        let low = 0;
        let high = g.numRows() - 1;

        while (low < high) {
            let mid = Math.floor((low + high) / 2);
            let diff = g.getValue(mid, 0) - time;

            if (diff < 0) {
                low = mid + 1;
            } else if (diff > 0) {
                high = mid - 1;
            } else {
                return mid;
            }
        }

        return low;
    }

    findStateBlocks(g, start, end) {
        "use strict";
        var result = [];
        var state = this.state[start]; //getState(g, start);             // current state
        var newState;
        for (var i = start; i < end; i++) {
            // find the next change
            newState = this.state[i]; //getState(g, i);
            if (newState !== state) {
                result.push({
                    row: i,
                    state: state,
                });
                state = newState;
            }
        }
        result.push({
            row: end,
            state: state,
        });
        return result;
    }

    getTime(g, row) {
        "use strict";
        if (row >= g.numRows()) {
            row = g.numRows() - 1;
        }
        return g.getValue(row, 0);
    }

    drawBackground(ctx, area, graph) {
        var timeStart = graph.toDataXCoord(area.x);
        var timeEnd = graph.toDataXCoord(area.x + area.w);
        // the data rows for the range we are interested in. 0-based index. This is deliberately extended out one row
        // to be sure the range is included
        var rowStart = Math.max(this.findNearestRow(graph, timeStart) - 1, 0);
        var rowEnd = this.findNearestRow(graph, timeEnd) + 1;
        if (rowStart === null || rowEnd === null) {
            return;
        }
        var blocks = this.findStateBlocks(graph, rowStart, rowEnd); // rowEnd is exclusive

        var startX = 0; // start drawing from 0 - the far left
        for (var i = 0; i < blocks.length; i++) {
            var block = blocks[i];
            var row = block.row; // where this state run ends
            var t = this.getTime(graph, row); // convert to time. Using time ensures the display matches the plotted resolution
            // of the graph.
            var r = (t - timeStart) / (timeEnd - timeStart); // as a fraction of the entire display
            var endX = Math.floor(area.x + area.w * r);

            var state = STATES[parseInt(block.state, 10)];
            if (state === undefined) {
                state = STATES[0];
            }
            //var borderColor = (state.waiting || state.extending) ? setAlphaFactor(state.color, 0.5) : undefined;
            //var bgColor = (state.waiting) ? bgColor = colorIdle : state.color;
            ctx.fillStyle = state.color;
            ctx.fillRect(
                startX,
                area.h - STATE_LINE_WIDTH,
                endX - startX,
                area.h,
            );
            startX = endX;
        }
    }

    addResume() {
        this.anno.push({
            series: "beerTemp",
            x: this.ctime * 1000,
            shortText: "R",
            text: "Resume",
            attachAtBottom: true,
        });
    }

    getXRange() {
        if (typeof this.chart == "undefined") return [0, 0];
        return this.chart.xAxisRange();
    }

    setXRange(range) {
        if (typeof this.chart == "undefined") return;
        this.chart.updateOptions({ dateWindow: range });
    }

    partial(start, end) {
        var me = this;

        var srow = me.findNearestRow(this.chart, start);
        var erow = me.findNearestRow(this.chart, end);
        var data = [];
        var VERTAG = 5;
        var st = Math.round(start / 1000);

        // create a header
        // header
        var tag = VERTAG | (me.celsius ? 0xe0 : 0xf0);
        if (me.calibrating) tag = tag ^ 0x20;
        // headerx4
        var header = new Uint8Array([
            0xff,
            tag,
            me.interval >> 8,
            me.interval & 0xff,
            (st >> 24) & 0xff,
            (st >> 16) & 0xff,
            (st >> 8) & 0xff,
            st & 0xff,
        ]);

        //  Period Record x 2
        data.push(header);
        function encodeTemp(t) {
            // valid temp range, 225 ~ -100
            // 0 ~ 225:
            // -100 ~ 0 :  226  - t  , maximum 32500 ( max uint16 32767)
            if (isNaN(t)) return [0x7f, 0xff];
            var it = parseInt(t * 100);
            if (it < 0) it = 22500 - it;
            return [(it >> 8) & 0x7f, it & 0xff];
        }

        function encodeGravity(g) {
            if (isNaN(g)) return [0x7f, 0xff];
            var sgint = Math.round(g * 10000);
            return [(sgint >> 8) & 0xff, sgint & 0xff];
        }

        function encodeTilt(tlt) {
            //        if(isNaN(tlt)) return [0x7F,0xFF];
            var tltInt = tlt * 100;
            return [(tltInt >> 8) & 0xff, tltInt & 0xff];
        }
        var values = [null, null, null, null, null, null, null, null];

        function periodRecord(row) {
            var rec = [0xf0, 0x00];
            var currentValues = [
                me.chart.getValue(row, LineIndex.BeerSet),
                me.chart.getValue(row, LineIndex.BeerTemp),
                me.chart.getValue(row, LineIndex.FridgeTemp),
                me.chart.getValue(row, LineIndex.FridgeSet),
                me.chart.getValue(row, LineIndex.RoomTemp),
                me.chart.getValue(row, LineIndex.AuxTemp),
                me.rawSG[row],
                me.angles[row],
            ];
            var mask = 0;

            for (var i = 0; i < 8; i++) {
                if (values[i] != currentValues[i]) {
                    mask = mask | (1 << i);
                    if (i < 5) {
                        rec = rec.concat(encodeTemp(currentValues[i]));
                    } else {
                        if (i == 5) {
                            var auxtemp = currentValues[5];
                            if (auxtemp && !isNaN(auxtemp)) {
                                rec = rec.concat(encodeTemp(auxtemp));
                            }
                        } else if (i == 6) {
                            // gravity
                            // The gravity data is user-input or calculated by BPL.
                            var sg = currentValues[6];
                            if (sg && !isNaN(sg)) {
                                rec = rec.concat(encodeGravity(sg));
                            }
                        } else {
                            var tilt = currentValues[7];
                            if (tilt && !isNaN(tilt)) {
                                rec = rec.concat(encodeTilt(tilt));
                            }
                        }
                        currentValues[i] = null;
                    }
                }
            }
            rec[1] = mask;
            values = currentValues;
            return rec;
        }
        data.push(new Uint8Array(periodRecord(srow)));
        // mode y state
        var brewpistate = me.state[srow];
        data.push(
            new Uint8Array([
                0xf4,
                me.getModeBeforeTime(start),
                0xf1,
                brewpistate,
            ]),
        );
        // OG, if exists
        if (!isNaN(me.og)) {
            var og = [0xf8, 0];
            data.push(new Uint8Array(og.concat(encodeGravity(me.og))));
        }
        // tilt in water
        if (me.calibrating) {
            var twa = [0xf9, 0];
            data.push(new Uint8Array(twa.concat(encodeTilt(me.tiltInWater))));
        }
        var anno = this.anno;
        var aidx = 0;

        while (aidx < anno.length && anno[aidx].x <= start) aidx++;

        for (var r = srow + 1; r <= erow; r++) {
            // check annotation.
            var time = me.data[r][0].getTime();
            if (
                aidx < anno.length &&
                time >= anno[aidx].x &&
                anno[aidx].shortText == "R"
            ) {
                const tdiff = Math.round((time - start) / 1000);
                data.push(
                    new Uint8Array([
                        0xfe,
                        (tdiff >> 16) & 0xff,
                        (tdiff >> 8) & 0xff,
                        tdiff && 0xff,
                    ]),
                );
                aidx++;
            }

            data.push(new Uint8Array(periodRecord(r)));
            if (brewpistate != me.state[r]) {
                brewpistate = me.state[r];
                data.push(new Uint8Array([0xf1, brewpistate]));
            }

            if (
                aidx < anno.length &&
                time >= anno[aidx].x &&
                anno[aidx].shortText != "R"
            ) {
                // mode
                data.push(
                    new Uint8Array([0xf4, anno[aidx].shortText.charCodeAt(0)]),
                );
                aidx++;
            }
        }
        return data;
    }

    addMode(m, x) {
        const s = String.fromCharCode(m);
        this.anno.push({
            series: "beerTemp",
            x: x,
            shortText: s.toUpperCase(),
            text: ModeMap[s],
            attachAtBottom: true,
        });
    }
}
