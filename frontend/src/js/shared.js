export const JSVERSION = "3.6";

export function s_ajax(b) {
    var c = new XMLHttpRequest();
    c.onreadystatechange = function() {
        if (c.readyState == 4) {
            if (c.status >= 200 && c.status < 300) {
                b.success(c.responseText)
            } else {
                if (typeof b.error === 'function') {
                    b.error(c.status, c.statusText, c.responseText);
                }
            }
        }
    };
    c.ontimeout = function() {
        if (typeof b["timeout"] != "undefined") b.timeout();
        else c.onerror(-1)
    }, c.onerror = function(a) {
        if (typeof b["fail"] != "undefined") b.fail(a)
    };
    c.open(b.m, b.url, true);
    if (typeof b["data"] != "undefined") {
        c.setRequestHeader("Content-Type", (typeof b["mime"] != "undefined") ? b["mime"] : "application/x-www-form-urlencoded");
        c.send(b.data)
    } else c.send()
}

export let Q = function(d) {
    return document.querySelector(d);
};

export function C2F(c) {
    return Math.round((c * 1.8 + 32) * 10) / 10
}

export function F2C(f) {
    return Math.round((f - 32) / 1.8 * 10) / 10
}

export function openDlgLoading() {
    document.getElementById('dlg_loading').style.display = "block";
}

export function closeDlgLoading() {
    document.getElementById('dlg_loading').style.display = "none";
}

export var BrewMath = {
    abv: function(og, fg) {
        return ((76.08 * (og - fg) / (1.775 - og)) * (fg / 0.794)).toFixed(1);
    },
    abvP: function(og, fg) {
        return BrewMath.abv(BrewMath.pla2sg(og), BrewMath.pla2sg(fg));
    },
    att: function(og, fg) {
        return Math.round((og - fg) / (og - 1) * 100);
    },
    attP: function(pog, pfg) {
        return Math.round((pog - pfg) / pog * 100);
    },
    sg2pla: function(sg) {
        return (((182.4601 * sg - 775.6821) * sg + 1262.7794) * sg - 669.5622);
    },
    pla2sg: function(pla) {
        return 1 + (pla / (258.6 - ((pla / 258.2) * 227.1)));
    },
    tempCorrectionF(sg, t, c) {
        return sg * ((1.00130346 - 0.000134722124 * t + 0.00000204052596 * t * t - 0.00000000232820948 * t * t * t) /
            (1.00130346 - 0.000134722124 * c + 0.00000204052596 * c * c - 0.00000000232820948 * c * c * c));
    },
    pTempCorrectionF(sg, t, c) {
        return BrewMath.sg2pla(BrewMath.tempCorrectionF(BrewMath.pla2sg(sg), t, c));
    },
    tempCorrection(celsius, sg, t, c) {
        return celsius ? BrewMath.tempCorrectionF(sg, C2F(t), C2F(c)) : BrewMath.tempCorrectionF(sg, t, c);
    },
    pTempCorrection(celsius, sg, t, c) {
        return celsius ? BrewMath.pTempCorrectionF(sg, C2F(t), C2F(c)) : BrewMath.tempCorrectionF(sg, t, c);
    }
};

Date.prototype.shortLocalizedString = function() {
    var y = this.getYear() + 1900;
    var re = new RegExp('[^\d]?' + y + '[^\d]?');
    var n = this.toLocaleDateString();
    var ds = n.replace(re, "");
    var HH = this.getHours();
    var MM = this.getMinutes();

    function T(x) {
        return (x > 9) ? x : ("0" + x);
    }
    return ds + " " + T(HH) + ":" + T(MM);
};

export function getActiveNavItem() {
    var path = window.location.pathname.split("/").pop();
    if (path == "") path = "index.htm";
    var element = Q('.options>li>a[href="/' + path + '"]');
    element.className += 'active';
}

function dd(n) { return (n < 10) ? '0' + n : n; }

export function formatDate(dt) {
    //	var y = dt.getFullYear();
    //	var M = dt.getMonth() +1;
    //	var d = dt.getDate();
    var h = dt.getHours();
    var m = dt.getMinutes();
    //    var s = dt.getSeconds();

    //	return dd(M) + "/" + dd(d) + "/" + y +" "+ dd(h) +":"+dd(m)+":"+dd(s);
    //	return dd(M) + "/" + dd(d) +" "+ dd(h) +":"+dd(m);
    return dt.toLocaleDateString() + " " + dd(h) + ":" + dd(m);
}

export function formatDateForPicker(date) {
    var h = date.getHours();
    var m = date.getMinutes();

    return date.getFullYear() + "-" + dd(date.getMonth() + 1) + "-" + dd(date.getDate()) + "T" + dd(h) + ":" + dd(m);
}

export function updateGravity(sg) {
    //if(typeof window.sg != "undefined") return;
    window.sg = sg;
    Q("#gravity-sg").innerHTML = window.plato ? sg.toFixed(1) : sg.toFixed(3);
    if (typeof window.og != "undefined") {
        Q("#gravity-att").innerHTML = window.plato ? BrewMath.attP(window.og, sg) : BrewMath.att(window.og, sg);
        Q("#gravity-abv").innerHTML = window.plato ? BrewMath.abvP(window.og, sg) : BrewMath.abv(window.og, sg);
    }
}

export function updateOriginGravity(og) {
    if (typeof window.og != "undefined" && window.og == og) return;
    window.og = og;
    Q("#gravity-og").innerHTML = window.plato ? og.toFixed(1) : og.toFixed(3);
    if (typeof window.sg != "undefined")
        updateGravity(window.sg);
}

export const ModeString = {
    o: "<%= mode_off %>",
    b: "<%= mode_beer_const %>",
    f: "<%= mode_fridge_const %>",
    p: "<%= mode_beer_profile %>",
    i: "Invalid"
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
    "<%= state_text_invalid %>"
];
