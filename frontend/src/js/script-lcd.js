import { byId, ModeString, StateText } from "./shared";
import { BWF } from "./vendor/bwf";

function genStateText(state, duration) {
    if (state == 1 || state == 2 || state == 10 || state == 7) return StateText[state];

    var timestr = "";
    var mm = Math.floor(duration / 60);
    var hh = Math.floor(mm / 60);
    var ss = duration % 60;
    mm = mm - hh * 60;

    function zeropad(n){
        return n>9? ""+n:"0"+n;
    }

    if (hh > 0) {
        timestr = "{HH}h{MM}m{SS}".replace("{SS}", zeropad(ss)).replace("{MM}", zeropad(mm)).replace("{HH}", zeropad(hh));
    } else{
        // short
        timestr = "{MM}m{SS}".replace("{SS}", zeropad(ss)).replace("{MM}", zeropad(mm));
    }
    return StateText[state].replace("{time}", timestr);
}
var roomOfridge = false;

function simLcd(info) {
    function showTemp(tp) {
        // always takes 4 chars
        if (tp < -10000) return " --.-";
        var text = (tp / 100.0).toFixed(1);
        var spaces = "";
        var i = text.length;
        for (; i < 5; i++) spaces += " ";
        return spaces + text;
    }

    var lines = [];
    lines[0] = "Mode   " + ModeString[info.md];
    lines[1] = "Beer  " + showTemp(info.bt) + " " + showTemp(info.bs) + " &deg;" + info.tu;
    if (info.rt > -10001 && roomOfridge)
        lines[2] = "Room  " + showTemp(info.rt) + " " + showTemp(-20000) + " &deg;" + info.tu;
    else
        lines[2] = "Fridge" + showTemp(info.ft) + " " + showTemp(info.fs) + " &deg;" + info.tu;
    roomOfridge = !roomOfridge;
    lines[3] = info.sl;
    return lines;
}

function displayLcdText(lines) {
    for (var i = 0; i < 4; i++) {
        var d = byId("lcd-line-" + i);
        if (d) d.innerHTML = lines[i];
    }
}

function setLcdText(id, html) {
    var d = byId(id);
    d.innerHTML = html;
}

function communicationError() {
    displayLcdText(["Failed to", "connect to", "Server", ""]);
}

function controllerError() {
    displayLcdText(["Controller not", "updating data", "...", ""]);
}

function resize() {
    var width = document.documentElement.clientWidth - 20;
    var height = (width - 10) / 2.8;
    var fontsize = height / 4 - 10;
    var frame = document.getElementsByClassName("lcddisplay")[0];
    frame.style.width = width + "px";
    frame.style.height = height + "px";
    document.getElementsByClassName("lcd-text")[0].style.fontSize = parseInt(fontsize) + "px";
}

export function init() {
    resize();
    window.addEventListener("resize", function() {
        resize();
    }, false);
    byId("lcd").onclick = function() {
        byId("myDropdown").classList.toggle("show");
        event.stopPropagation();
    };

    var gotMsg = true;
    BWF.init({
        onconnect: function() {},
        error: function(e) {
            console.log("error");
            communicationError();
        },
        handlers: {
            A: function(info) {
                if (typeof info["sl"] != "undefined") {
                    displayLcdText(simLcd(info));
                }
            }
        }
    });
}

window.onclick = function(event) {
    const dropdowns = document.getElementsByClassName("dropdown-content");
    for (let i = 0; i < dropdowns.length; i++) {
        var openDropdown = dropdowns[i];
        if (openDropdown.classList.contains('show')) {
            openDropdown.classList.remove('show');
        }
    }
}
