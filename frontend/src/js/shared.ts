export const JSVERSION = "3.6";

export const select = document.querySelector.bind(document);

interface AjaxOptions {
    url: string;
    m: string; // HTTP method: "GET", "POST", etc.
    data?: Document | XMLHttpRequestBodyInit | null | undefined;
    mime?: string;
    success: (response: string) => void;
    error?: (status: number, statusText: string, response: string) => void;
    timeout?: () => void;
    fail?: (event: ProgressEvent | number) => void;
}

export function s_ajax(b: AjaxOptions) {
    const c = new XMLHttpRequest();

    c.onreadystatechange = () => {
        if (c.readyState == 4) {
            if (c.status >= 200 && c.status < 300) {
                b.success(c.responseText);
            } else if (typeof b.error === "function") {
                b.error(c.status, c.statusText, c.responseText);
            }
        }
    };

    c.ontimeout = () => {
        if (typeof b.timeout === "function") {
            b.timeout();
        } else if (typeof b.fail === "function") {
            b.fail(-1);
        }
    };

    c.onerror = (a: ProgressEvent) => {
        if (typeof b.fail === "function") {
            b.fail(a);
        }
    };

    c.open(b.m, b.url, true);

    if (typeof b.data !== "undefined") {
        const contentType = b.mime ?? "application/x-www-form-urlencoded";
        c.setRequestHeader("Content-Type", contentType);
        c.send(b.data);
    } else {
        c.send();
    }
}

export function C2F(c: number) {
    return Math.round((c * 1.8 + 32) * 10) / 10;
}

export function F2C(f: number) {
    return Math.round(((f - 32) / 1.8) * 10) / 10;
}

export function openDlgLoading(): void {
    const dlg = document.getElementById("dlg_loading");
    if (dlg) dlg.style.display = "block";
}

export function closeDlgLoading(): void {
    const dlg = document.getElementById("dlg_loading");
    if (dlg) dlg.style.display = "none";
}

export var BrewMath = {
    abv: function (og: number, fg: number) {
        return (((76.08 * (og - fg)) / (1.775 - og)) * (fg / 0.794)).toFixed(1);
    },
    abvP: function (og: number, fg: number) {
        return BrewMath.abv(BrewMath.pla2sg(og), BrewMath.pla2sg(fg));
    },
    att: function (og: number, fg: number) {
        return Math.round(((og - fg) / (og - 1)) * 100);
    },
    attP: function (pog: number, pfg: number) {
        return Math.round(((pog - pfg) / pog) * 100);
    },
    sg2pla: function (sg: number) {
        return ((182.4601 * sg - 775.6821) * sg + 1262.7794) * sg - 669.5622;
    },
    pla2sg: function (pla: number) {
        return 1 + pla / (258.6 - (pla / 258.2) * 227.1);
    },
    tempCorrectionF(sg: number, t: number, c: number) {
        return (
            sg *
            ((1.00130346 -
                    0.000134722124 * t +
                    0.00000204052596 * t * t -
                    0.00000000232820948 * t * t * t) /
                (1.00130346 -
                    0.000134722124 * c +
                    0.00000204052596 * c * c -
                    0.00000000232820948 * c * c * c))
        );
    },
    pTempCorrectionF(sg: number, t: number, c: number) {
        return BrewMath.sg2pla(BrewMath.tempCorrectionF(BrewMath.pla2sg(sg), t, c));
    },
    tempCorrection(celsius: boolean, sg: number, t: number, c: number) {
        return celsius
            ? BrewMath.tempCorrectionF(sg, C2F(t), C2F(c))
            : BrewMath.tempCorrectionF(sg, t, c);
    },
    pTempCorrection(celsius: boolean, sg: number, t: number, c: number) {
        return celsius
            ? BrewMath.pTempCorrectionF(sg, C2F(t), C2F(c))
            : BrewMath.tempCorrectionF(sg, t, c);
    },
};

declare global {
    interface Date {
        shortLocalizedString(): string;
    }
}

Date.prototype.shortLocalizedString = function (): string {
    var y = this.getFullYear();
    var re = new RegExp("[^d]?" + y + "[^d]?");
    var n = this.toLocaleDateString();
    const dateString = n.replace(re, "");
    const hours = this.getHours().toString().padStart(2, "0");
    const minutes = this.getMinutes().toString().padStart(2, "0");

    return `${dateString} ${hours}:${minutes}`;
};

export function getActiveNavItem() {
    var path = window.location.pathname.split("/").pop();
    if (path == "") path = "index.htm";
    var element = select('.options>li>a[href="/' + path + '"]');
    if (element) element.className += "active";
}

function dd(n: number): string {
    return n < 10 ? "0" + n : n.toString();
}

export function formatDate(date: Date) {
    const hours = date.getHours().toString().padStart(2, "0");
    const minutes = date.getMinutes().toString().padStart(2, "0");

    return `${date.toLocaleDateString()} ${hours}:${minutes}`;
}

export function formatDateForPicker(date: Date) {
    var h = date.getHours();
    var m = date.getMinutes();

    return (
        date.getFullYear() +
        "-" +
        dd(date.getMonth() + 1) +
        "-" +
        dd(date.getDate()) +
        "T" +
        dd(h) +
        ":" +
        dd(m)
    );
}

export function updateGravity(sg: number): void {
    window.sg = sg;

    const gravitySg = select("#gravity-sg");
    if (gravitySg) {
      gravitySg.innerHTML = window.plato ? sg.toFixed(1) : sg.toFixed(3);
    }

    if (typeof window.og !== "undefined") {
      const gravityAtt = select("#gravity-att");
      const gravityAbv = select("#gravity-abv");

        if (gravityAtt) {
            gravityAtt.innerHTML = window.plato
                ? BrewMath.attP(window.og, sg).toString()
                : BrewMath.att(window.og, sg).toString();
        }

        if (gravityAbv) {
            gravityAbv.innerHTML = window.plato
                ? BrewMath.abvP(window.og, sg)
                : BrewMath.abv(window.og, sg);
        }
    }
}

export function updateOriginGravity(og: number): void {
    if (typeof window.og !== "undefined" && window.og === og) return;
    window.og = og;

    const gravityOg = select("#gravity-og");
    if (gravityOg) {
        gravityOg.innerHTML = window.plato ? og.toFixed(1) : og.toFixed(3);
    }
    if (typeof window.sg !== "undefined") updateGravity(window.sg);
}

export const ModeString = {
    o: "<%= mode_off %>",
    b: "<%= mode_beer_const %>",
    f: "<%= mode_fridge_const %>",
    p: "<%= mode_beer_profile %>",
    i: "Invalid",
};

export const StateText = [
    "<%= state_text_idle %>",
    "<%= state_text_off %>",
    "<%= state_text_door_Open %>",
    "<%= state_text_heating %>",
    "<%= state_text_cooling %>",
    "<%= state_text_wait_to_cool %>",
    "<%= state_text_wait_to_heat %>",
    "<%= state_text_wait_for_peak %>",
    "<%= state_text_cooling_min_time %>",
    "<%= state_text_heating_min_time %>",
    "<%= state_text_invalid %>",
];
