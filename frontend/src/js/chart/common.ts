import { select } from "../shared";
import { gravityTracker } from "./GravityTracker";

export function testData(
    data: number[],
): false | { sensor: number; f: number } {
    if (data[0] != 0xff) return false;

    const s = data[1] & 0x07;
    if (s != 5) return false;

    return {
        sensor: s,
        f: data[1] & 0x10,
    };
}

function fgstate(duration: number): void {
    const Color: Record<number, string> = {
        0: "red",
        12: "orange",
        24: "yellow",
        48: "green",
    };
    const el = select("#fgstate") as HTMLElement;
    el.style.backgroundColor = Color[duration];
}

export function checkfgstate(): void {
    if (gravityTracker.stable(48)) {
        fgstate(48);
    } else if (gravityTracker.stable(24)) {
        fgstate(24);
    } else if (gravityTracker.stable(12)) {
        fgstate(12);
    } else {
        fgstate(0);
    }
}

const colorIdle = "white";
const colorCool = "rgba(0, 0, 255, 0.4)";
const colorHeat = "rgba(255, 0, 0, 0.4)";
const colorWaitingHeat = "rgba(255, 0, 0, 0.2)";
const colorWaitingCool = "rgba(0, 0, 255, 0.2)";
const colorHeatingMinTime = "rgba(255, 0, 0, 0.6)";
const colorCoolingMinTime = "rgba(0, 0, 255, 0.6)";
const colorWaitingPeakDetect = "rgba(0, 0, 0, 0.2)";

export const STATE_LINE_WIDTH = 15;

export interface StateDefinition {
    name: string;
    color: string;
    text: string;
    doorOpen?: boolean;
    waiting?: boolean;
    extending?: boolean;
}

export const STATES: StateDefinition[] = [
    {
        name: "IDLE",
        color: colorIdle,
        text: "<%= chart_state_idle %>",
    },
    {
        name: "STATE_OFF",
        color: colorIdle,
        text: "<%= chart_state_off %>",
    },
    {
        name: "DOOR_OPEN",
        color: "#eee",
        text: "<%= chart_state_door_Open %>",
        doorOpen: true,
    },
    {
        name: "HEATING",
        color: colorHeat,
        text: "<%= chart_state_heating %>",
    },
    {
        name: "COOLING",
        color: colorCool,
        text: "<%= chart_state_cooling %>",
    },
    {
        name: "WAITING_TO_COOL",
        color: colorWaitingCool,
        text: "<%= chart_state_wait_to_cool %>",
        waiting: true,
    },
    {
        name: "WAITING_TO_HEAT",
        color: colorWaitingHeat,
        text: "<%= chart_state_wait_to_heat %>",
        waiting: true,
    },
    {
        name: "WAITING_FOR_PEAK_DETECT",
        color: colorWaitingPeakDetect,
        text: "<%= chart_state_wait_for_peak %>",
        waiting: true,
    },
    {
        name: "COOLING_MIN_TIME",
        color: colorCoolingMinTime,
        text: "<%= chart_state_cooling_min_time %>",
        extending: true,
    },
    {
        name: "HEATING_MIN_TIME",
        color: colorHeatingMinTime,
        text: "<%= chart_state_heating_min_time %>",
        extending: true,
    },
    {
        name: "INVALID",
        color: colorHeatingMinTime,
        text: "<%= chart_state_invalid %>",
    },
];
