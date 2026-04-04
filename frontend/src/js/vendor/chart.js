import { select } from "../shared";

// gravity tracking
export var GravityFilter = {
    b: 0.1,
    y: 0,
    reset: function() {
        this.y = 0;
    },
    add: function(gravity) {
        if (this.y == 0) this.y = gravity;
        else this.y = this.y + this.b * (gravity - this.y);
        return Math.round(this.y * 10000) / 10000;
    },
    setBeta: function(beta) {
        this.b = beta;
    }
};
export var GravityTracker = {
    NumberOfSlots: 48,
    InvalidValue: 0xFF,
    ridx: 0,
    record: [],
    threshold: 1,
    setThreshold: function(t) {
        this.threshold = t;
    },
    addRecord: function(v) {
        this.record[this.ridx++] = v;
        if (this.ridx >= this.NumberOfSlots) this.ridx = 0;
    },
    stable: function(duration, to) {
        to = (typeof to == "undefined") ? this.threshold : to;
        var current = this.ridx - 1;
        if (current < 0) current = this.NumberOfSlots - 1;
        var previous = this.NumberOfSlots + this.ridx - duration;
        while (previous >= this.NumberOfSlots) previous -= this.NumberOfSlots;
        return (this.record[previous] - this.record[current]) <= to;
    },
    Period: 60 * 60,
    init: function() {
        this.curerntStart = 0;
        this.lastValue = 0;
    },
    add: function(gravity, time) {
        //gravity = Math.round(fgravity * 1000, 1);
        var timediff = time - this.curerntStart;

        if (timediff > this.Period) {
            this.addRecord(gravity);
            if (this.lastValue != 0) {
                timediff -= this.Period;
                while (timediff > this.Period) {
                    timediff -= this.Period;
                    this.addRecord(this.lastValue);
                }
            }
            this.curerntStart = time;
            this.lastValue = gravity;
        }
    }
};

export function testData(data) {
    if (data[0] != 0xFF) return false;
    var s = data[1] & 0x07;
    if (s != 5) return false;

    return {
        sensor: s,
        f: data[1] & 0x10
    };
};

function fgstate(duration) {
    var Color = {
        0: "red",
        12: "orange",
        24: "yellow",
        48: "green"
    };
    select("#fgstate").style.backgroundColor = Color[duration];
}

export function checkfgstate() {
    if (GravityTracker.stable(12)) {
        if (GravityTracker.stable(24)) {
            if (GravityTracker.stable(48)) fgstate(48);
            else fgstate(24); // 24
        } else fgstate(12); //
    } else fgstate(0);
}
// gravity tracking
var GravityIndex = 6;
var TiltAngleIndex = 7;
var RoomTemperatureIndex = 4;

var colorIdle = "white";
var colorCool = "rgba(0, 0, 255, 0.4)";
var colorHeat = "rgba(255, 0, 0, 0.4)";
var colorWaitingHeat = "rgba(255, 0, 0, 0.2)";
var colorWaitingCool = "rgba(0, 0, 255, 0.2)";
var colorHeatingMinTime = "rgba(255, 0, 0, 0.6)";
var colorCoolingMinTime = "rgba(0, 0, 255, 0.6)";
var colorWaitingPeakDetect = "rgba(0, 0, 0, 0.2)";
export var STATE_LINE_WIDTH = 15;
export var STATES = [{
    name: "IDLE",
    color: colorIdle,
    text: "<%= chart_state_idle %>"
}, {
    name: "STATE_OFF",
    color: colorIdle,
    text: "<%= chart_state_off %>"
}, {
    name: "DOOR_OPEN",
    color: "#eee",
    text: "<%= chart_state_door_Open %>",
    doorOpen: true
}, {
    name: "HEATING",
    color: colorHeat,
    text: "<%= chart_state_heating %>"
}, {
    name: "COOLING",
    color: colorCool,
    text: "<%= chart_state_cooling %>"
}, {
    name: "WAITING_TO_COOL",
    color: colorWaitingCool,
    text: "<%= chart_state_wait_to_cool %>",
    waiting: true
}, {
    name: "WAITING_TO_HEAT",
    color: colorWaitingHeat,
    text: "<%= chart_state_wait_to_heat %>",
    waiting: true
}, {
    name: "WAITING_FOR_PEAK_DETECT",
    color: colorWaitingPeakDetect,
    text: "<%= chart_state_wait_for_peak %>",
    waiting: true
}, {
    name: "COOLING_MIN_TIME",
    color: colorCoolingMinTime,
    text: "<%= chart_state_cooling_min_time %>",
    extending: true
}, {
    name: "HEATING_MIN_TIME",
    color: colorHeatingMinTime,
    text: "<%= chart_state_heating_min_time %>",
    extending: true
}, {
    name: "INVALID",
    color: colorHeatingMinTime,
    text: "<%= chart_state_invalid %>"
}];
