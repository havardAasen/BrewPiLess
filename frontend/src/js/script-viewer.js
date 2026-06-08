import { select, showPlatoUnit } from "./shared";
import { Labels } from "./chart/constants";
import { testData, STATES } from "./chart/common";
import { BrewChart } from "./chart/BrewChart";
import { registerChartControls } from "./chart/ChartControl";

/** @type BrewChart */
var bChart;

export function loaded() {
    function openfile(f) {
        if (f) {
            var r = new FileReader();
            r.onload = function (e) {
                window.file = f;
                //chart.clear();
                var data = new Uint8Array(e.target.result);
                if (testData(data) !== false) {
                    bChart.raw = data;
                    bChart.process(data);
                    if (bChart.calibrating) {
                        bChart.getFormula();
                        //  do it again
                        bChart.process(data);
                        if (bChart.calculateSG)
                            select("#formula-btn").style.display = "block";
                    }
                    bChart.updateChart();
                    var date = new Date(bChart.starttime * 1000);
                    select("#log-start").innerHTML = bChart.formatDate(date);
                    if (bChart.plato) showPlatoUnit();
                } else {
                    alert("<%= script_viewer_invalid_log %>");
                }
            };
            r.readAsArrayBuffer(f);
        } else {
            alert("<%= script_viewer_failed_load_file %>");
        }
    }

    bChart = new BrewChart(
        "div_g",
        select("#ylabel").innerHTML,
        select("#y2label").innerHTML,
    );
    registerChartControls(bChart);

    if (select("#dropfile")) {
        select("#dropfile").ondragover = function (e) {
            e.stopPropagation();
            e.preventDefault();
            e.dataTransfer.dropEffect = "copy"; // Explicitly show this is a copy.
        };

        select("#dropfile").ondrop = function (e) {
            e.stopPropagation();
            e.preventDefault();
            var f = e.dataTransfer.files[0];
            openfile(f);
        };
    }
    select("#fileinput").onchange = function (evt) {
        //Retrieve the first (and only!) File from the FileList object
        var f = evt.target.files[0];
        openfile(f);
    };
}

function download(blob, file) {
    var link = document.createElement("a");

    if (link.download === undefined) {
        // feature detection
        alert("<%= script_viewer_not_downloading_file %>");
        return;
    }

    var url = URL.createObjectURL(blob);
    link.setAttribute("href", url);
    link.setAttribute("download", file);
    link.style.visibility = "hidden";
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
}

function exportcsv() {
    if (typeof window.file == "undefined") return;
    // generate data
    var csv = "Time, Unix Time";
    for (let i = 1; i < Labels.length; i++) {
        csv = csv + (i == 0 ? "" : ",") + Labels[i];
    }
    csv = csv + ",Tilt,state\n";

    for (var row = 0; row < bChart.data.length; row++) {
        for (let i = 0; i < Labels.length; i++) {
            var v = bChart.chart.getValue(row, i);
            if (v === null) v = "";
            else if (isNaN(v)) v = "";
            if (i == 0) {
                var d = new Date(v);
                csv = csv + d.toISOString() + "," + v / 1000;
            } else csv = csv + "," + v;
        }
        csv = csv + "," + bChart.angles[row];

        var state = parseInt(bChart.state[row]);
        var st = !isNaN(state) ? STATES[state].text : "";
        csv = csv + "," + st + "\n";
    }
    var blob = new Blob([csv], {
        type: "text/csv;",
    });
    // Browsers that support HTML5 download attribute
    download(blob, window.file.name + ".csv");
}

function cutrange() {
    if (typeof window.file == "undefined") return;
    var ranges = bChart.chart.xAxisRange();
    var data = bChart.partial(ranges[0], ranges[1]);
    download(
        new Blob(data, { type: "octet/stream" }),
        window.file.name + "-partial",
    );
}

window.cutrange = cutrange;
window.exportcsv = exportcsv;
