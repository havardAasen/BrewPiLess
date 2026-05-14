import { select, showPlatoUnit } from "./shared";
import { testData } from "./chart/common";
import { BChart } from "./chart/BrewChartWrapper";
import { registerChartControls } from "./chart/ChartControl";

function getParameterByName(name, url) {
    if (!url) url = window.location.href;
    name = name.replace(/[\[\]]/g, "\\$&");
    var regex = new RegExp("[?&]" + name + "(=([^&#]*)|&|#|$)"),
        results = regex.exec(url);
    if (!results) return null;
    if (!results[2]) return "";
    return decodeURIComponent(results[2].replace(/\+/g, " "));
}

function dataUrl() {
    return getParameterByName("log");
}

function getRangeURL(range) {
    var re = /([^\?]+)/;
    var result = window.location.href.match(re);
    return (
        result[1] +
        "?log=" +
        encodeURIComponent(getParameterByName("log")) +
        "&r=" +
        range[0] +
        "-" +
        range[1]
    );
}

function getFilename() {
    var log = getParameterByName("log");
    var re = /.([^\/]+)$/;
    var matches = re.exec(log);
    if (matches) return matches[1];
    return log;
}

export function loaded() {
    // get range, if any
    var range = getParameterByName("r");
    if (range) {
        window.iniRange = range.split("-");
    }
    BChart.init(
        "div_g",
        select("#ylabel").innerHTML,
        select("#y2label").innerHTML,
    );
    registerChartControls();

    select("#div_g").oncontextmenu = function (ev) {
        ev = ev || window.event;

        select("#myDropdown").classList.toggle("show");
        select("#myDropdown").style.left = ev.clientX + "px";
        select("#myDropdown").style.top = ev.clientY + "px";
        ev.stopPropagation();
        ev.preventDefault();
        return false;
    };

    window.addEventListener("click", function () {
        var dd = select("#myDropdown");
        if (dd.classList.contains("show")) dd.classList.remove("show");
    });

    select("#open-selection").onclick = function () {
        var ranges = BChart.chart.getXRange();
        console.log("range:" + ranges[0] + "-" + ranges[1]);
        window.open(getRangeURL(ranges), "_blank");
        return false;
    };

    select("#viewlogname").innerHTML = getFilename();

    var xhr = new XMLHttpRequest();
    xhr.open("GET", dataUrl());
    //	xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
    //	xhr.setRequestHeader("Content-length", PD.length);
    xhr.responseType = "arraybuffer";
    xhr.onload = function () {
        if (this.status == 404) {
            console.log("error getting log data.");
            return;
        }
        // response is unsigned 8 bit integer
        var data = new Uint8Array(this.response);

        if (testData(data) !== false) {
            BChart.raw = data;
            BChart.chart.process(data);
            if (BChart.chart.calibrating) {
                BChart.chart.getFormula();
                //  do it again
                BChart.chart.process(data);
                if (BChart.chart.calculateSG)
                    select("#formula-btn").style.display = "block";
            }
            BChart.chart.updateChart();
            var date = new Date(BChart.chart.starttime * 1000);
            select("#log-start").innerHTML = BChart.chart.formatDate(date);
            if (typeof window.iniRange !== "undefined")
                BChart.chart.setXRange(window.iniRange);
            if (BChart.chart.plato) showPlatoUnit();
        } else {
            alert("<%= script_viewer_invalid_log %>");
        }
    };
    xhr.ontimeout = function () {
        console.error("Timeout!");
    };
    xhr.onerror = function () {
        console.log("error getting data.");
    };
    //console.log(PD);
    xhr.send();
}
