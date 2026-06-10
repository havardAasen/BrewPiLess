import {
    byId,
    select,
    formatDate,
    formatDateForPicker,
    openDlgLoading,
    JSVERSION,
    closeDlgLoading,
    F2C,
    C2F,
    updateOriginGravity,
    updateNavbarVersion,
} from "./shared";
import { communicationError } from "./shared";
import { Capper } from "./capper";
import { BWF } from "./bwf";
import { PTC } from "./ptc";
import { ProfileChart } from "./chart/ProfileChart";
import { del, get, post } from "./httpClient";

/** @typedef {("C" | "F")} TempUnit */
/** @typedef {"profile" | "beer" | "fridge" | "off"} TemperatureControlMode */

var BPURL = "/tschedule";
var MAX_STEP = 7;

/** @type ProfileChart */
var profileChart;

/* profile.js */
var profileEditor = {
    dirty: false,
    /** @type TempUnit */
    tempUnit: "C",
    /** @type HTMLTableRowElement | null */
    row: null,

    sd: new Date(),
    C_startday_Id: "#startdate",
    C_savebtn_Id: "savebtn",

    /** @type HTMLTableSectionElement | null */
    tableBody: null,

    /** @param {boolean} d */
    markdirty: function (d) {
        this.dirty = d;
        byId(this.C_savebtn_Id).innerHTML = d ? "Save*" : "Save";
    },
    getStartDate: function () {
        return this.sd;
    },

    /** @param {Date} d */
    setStartDate: function (d) {
        this.sd = d;
        var date_in = select(this.C_startday_Id);
        date_in.value =
            date_in.type == "datetime-local"
                ? formatDateForPicker(d)
                : formatDate(d);
    },
    startDayChange: function () {
        var nd = new Date(select(this.C_startday_Id).value);
        if (isNaN(nd.getTime())) {
            // console.log("invalid date");
            this.setStartDate(this.sd);
        } else {
            // console.log(nd);
            this.sd = nd;
            this.reorg();
            this.markdirty(true);
        }
    },
    startnow: function () {
        var d = new Date();
        this.setStartDate(d);
        this.reorg();
        this.markdirty(true);
        profileChart.update(this.chartdata(), this.tempUnit);
    },

    /** @returns HTMLTableRowElement[] */
    getRows: function () {
        return Array.from(this.tableBody.getElementsByTagName("tr"));
    },
    sgChange: function (td) {
        if (
            !isNaN(td.innerHTML) ||
            td.innerHTML.match(/^[\d]+%$/) ||
            td.innerHTML == ""
        ) {
            td.saved = td.innerHTML;
            this.markdirty(true);
        } else {
            td.innerHTML = td.saved;
        }
    },
    dayChange: function (td) {
        if (td.innerHTML == "" || isNaN(td.innerHTML)) td.innerHTML = td.saved;
        else {
            this.markdirty(true);
            this.reorg();
            profileChart.update(this.chartdata(), this.tempUnit);
        }
    },
    tempChange: function (td) {
        if (td.innerHTML == "" || isNaN(td.innerHTML)) td.innerHTML = td.saved;
        else {
            this.markdirty(true);
            profileChart.update(this.chartdata(), this.tempUnit);
        }
    },
    stableChange: function (td) {
        if (td.innerHTML.match(/^\s*(\d+)@(\d+)\s*$/)) {
            td.saved = td.innerHTML;
            this.markdirty(true);
        } else if (!isNaN(td.innerHTML)) {
            td.saved = parseInt(td.innerHTML);
            this.markdirty(true);
        } else {
            td.innerHTML = td.saved;
        }
    },
    initrow: function (tr, stage) {
        var b = this;
        // temp setting
        var type = stage.c;
        tr.type = type;
        var tdTemp = tr.getElementsByClassName("stage-temp")[0];

        if (type == "r") {
            tdTemp.innerHTML = "";
        } else {
            tdTemp.innerHTML = stage.t;
            tdTemp.contentEditable = true;
            tdTemp.onblur = function () {
                b.tempChange(this);
            };
            tdTemp.onfocus = function () {
                this.saved = this.innerHTML;
            };
        }
        // day setting
        var tdDay = tr.getElementsByClassName("stage-time")[0];
        tdDay.innerHTML = stage.d;
        tdDay.contentEditable = true;
        tdDay.onblur = function () {
            b.dayChange(this);
        };
        tdDay.onfocus = function () {
            this.saved = this.innerHTML;
        };

        // stable setting
        var tdStable = tr.getElementsByClassName("stage-stabletime")[0];
        // sg. only valid for hold
        var tdSG = tr.getElementsByClassName("stage-sg")[0];

        if (type == "r") {
            tdSG.innerHTML = "";
            tdStable.innerHTML = "";
        } else {
            tdSG.saved = stage.g;
            tdSG.innerHTML = typeof stage.g == "undefined" ? "" : stage.g;
            tdSG.contentEditable = true;
            tdSG.onblur = function () {
                b.sgChange(this);
            };
            tdSG.onfocus = function () {
                this.saved = this.innerHTML;
            };
            if (typeof stage.s == "undefined") tdStable.innerHTML = "";
            else
                tdStable.innerHTML =
                    typeof stage.x == "undefined"
                        ? stage.s
                        : stage.x + "@" + stage.s;
            tdStable.contentEditable = true;
            tdStable.onblur = function () {
                b.stableChange(this);
            };
            tdStable.onfocus = function () {
                this.saved = this.innerHTML;
            };
        }

        var forTime = tr.getElementsByClassName("for-time")[0];
        // condition, only valid for hold
        var conSel = tr.getElementsByClassName("condition")[0];
        /*
           <option value="t" 0>Time</option>
           <option value="g" 1>SG</option>
           <option value="s" 2>Stable</option>
           <option value="a" 3>Time & SG</option>
           <option value="o" 4>Time OR SG</option>
           <option value="u" 5>Time OR Stable</option>
           <option value="v" 6>Time & Stable</option>
            <option value="b" 7>SG OR Stable</option>
            <option value="x" 8>SG & Stable</option>
            <option value="w" 9>ALL</option>
            <option value="e" 10>Either</option>
        */
        var conditionIndex = {
            t: 0,
            g: 1,
            a: 3,
            s: 2,
            o: 4,
            u: 5,
            v: 6,
            b: 7,
            x: 8,
            w: 9,
            e: 10,
        };
        if (type == "r") {
            forTime.style.display = "block";
            conSel.style.display = "none";
        } else {
            conSel.value = stage.c;
            conSel.selectedIndex = conditionIndex[stage.c];

            forTime.style.display = "none";
            conSel.style.display = "block";
        }
    },

    /**
     * @param {number} diff
     * @returns {string}
     */
    datestr: function (diff) {
        var dt = new Date(this.sd.getTime() + Math.round(diff * 86400) * 1000);
        return formatDate(dt);
    },
    reorg: function () {
        var rowlist = this.getRows();
        var utime = this.sd.getTime();
        for (var i = 0; i < rowlist.length; i++) {
            var row = rowlist[i];
            row.className = i % 2 ? "odd" : "even";
            row.getElementsByClassName("diaplay-time")[0].innerHTML =
                formatDate(new Date(utime));
            var time = this.rowTime(row);
            utime += Math.round(time * 86400) * 1000;
        }
    },
    chartdata: function () {
        var rowlist = this.getRows();
        if (rowlist.length == 0) return [];

        var utime = this.sd.getTime();
        let row = rowlist[0];
        var start = this.rowTemp(row);

        let list = [];
        list.push([new Date(utime), start]);

        for (var i = 0; i < rowlist.length; i++) {
            row = rowlist[i];
            var temp;
            if (row.type == "r") {
                temp = this.rowTemp(rowlist[i + 1]);
            } else {
                temp = this.rowTemp(row);
            }
            utime += Math.round(this.rowTime(row) * 86400) * 1000;
            list.push([new Date(utime), temp]);
        }
        return list;
    },
    addRow: function () {
        var rowlist = this.getRows();

        if (rowlist.length >= MAX_STEP) {
            alert("<%= script_control_too_many_steps %>");
            return;
        }
        var stage;

        if (rowlist.length == 0) {
            var init = this.tempUnit == "C" ? 20 : 68;
            stage = {
                c: "t",
                t: init,
                d: 1,
                g: 1.01,
            };
        } else {
            var lastRow = rowlist[rowlist.length - 1];

            let tr = this.row.cloneNode(true);
            this.initrow(tr, {
                c: "r",
                d: 1,
            });
            this.tableBody.appendChild(tr);
            stage = {
                c: "t",
                t: this.rowTemp(lastRow),
                d: 1,
                g: "",
            };
        }

        let tr = this.row.cloneNode(true);
        this.initrow(tr, stage);
        this.tableBody.appendChild(tr);

        this.reorg();
        this.markdirty(true);
        profileChart.update(this.chartdata(), this.tempUnit);
    },
    delRow: function () {
        // delete last row
        let list = this.getRows();
        if (list.length == 0) return;
        var last = list[list.length - 1];

        if (list.length > 1) {
            var lr = list[list.length - 2];
            lr.parentNode.removeChild(lr);
        }

        last.parentNode.removeChild(last);

        this.markdirty(true);
        profileChart.update(this.chartdata(), this.tempUnit);
    },
    rowTemp: function (row) {
        return parseFloat(
            row.getElementsByClassName("stage-temp")[0].innerHTML,
        );
    },
    rowCondition: function (row) {
        return row.getElementsByClassName("condition")[0].value;
    },
    rowTime: function (row) {
        return parseFloat(
            row.getElementsByClassName("stage-time")[0].innerHTML,
        );
    },
    rowSg: function (row) {
        return row.getElementsByClassName("stage-sg")[0].saved;
    },
    rowSt: function (row) {
        var data = row.getElementsByClassName("stage-stabletime")[0].innerHTML;
        if (typeof data != "string") return data;
        var matches = data.match(/^\s*(\d+)@(\d+)\s*$/);
        if (matches) {
            return parseInt(matches[2]);
        } else {
            return parseInt(data);
        }
    },
    rowStsg: function (row) {
        var data = row.getElementsByClassName("stage-stabletime")[0].innerHTML;
        if (typeof data != "string") return false;
        var matches = data.match(/^\s*(\d+)@(\d+)\s*$/);
        if (matches) {
            return parseInt(matches[1]);
        } else {
            return false;
        }
    },
    renderRows: function (g) {
        if (typeof g.length == "undefined") console.log("error!");
        for (var f = 0; f < g.length; f++) {
            var c = this.row.cloneNode(true);
            this.initrow(c, g[f]);
            this.tableBody.appendChild(c);
        }
        this.reorg();
    },

    initable: function (c, e) {
        const table = document.getElementById("profile_t");
        if (!table) throw new Error("Table was not found");

        this.tableBody = table.getElementsByTagName("tbody")[0];
        if (!this.tableBody) throw new Error("Table has no <tbody>");

        this.setStartDate(e);

        if (!this.row) {
            this.row = this.getRows()[0];
        } else {
            this.clear();
        }
        this.renderRows(c);
    },
    clear: function () {
        var rl = this.getRows();

        for (var i = rl.length - 1; i >= 0; i--) {
            var tr = rl[i];
            tr.parentNode.removeChild(tr);
        }
        this.markdirty(true);
    },
    getProfile: function () {
        var rl = this.getRows();
        var temps = [];
        for (let i = 0; i < rl.length; i++) {
            var tr = rl[i];
            var day = this.rowTime(tr);
            if (isNaN(day)) return false;

            if (tr.type == "r") {
                temps.push({
                    c: "r",
                    d: day,
                });
            } else {
                var temp = this.rowTemp(tr);
                if (isNaN(temp)) return false;
                if (
                    temp > BrewPiSetting.maxDegree ||
                    temp < BrewPiSetting.minDegree
                )
                    return false;

                /*
                   <option value="t">Time</option>
                   <option value="g">SG</option>
                   <option value="s">Stable</option>
                   <option value="a">Time & SG</option>
                   <option value="o">Time OR SG</option>
                   <option value="u">Time OR Stable</option>
                   <option value="v">Time & Stable</option>
                    <option value="b">SG OR Stable</option>
                    <option value="x">SG & Stable</option>
                    <option value="w">ALL</option>
                    <option value="e">Either</option>
                */
                var condition = this.rowCondition(tr);
                var stage = {
                    c: condition,
                    d: day,
                    t: temp,
                };

                var useSg = "gaobxwe";
                var gv = this.rowSg(tr);

                if (useSg.indexOf(condition) >= 0) {
                    if (gv == "") return false;
                    stage.g = gv;
                }
                var useStableTime = "suvbxwe";
                var stv = this.rowSt(tr);
                if (useStableTime.indexOf(condition) >= 0) {
                    if (isNaN(stv)) return false;
                    stage.s = stv;
                    var x = this.rowStsg(tr);
                    if (x) stage.x = x;
                }

                temps.push(stage);
            }
        }
        var s = this.sd.toISOString();
        var ret = {
            s: s,
            v: 2,
            u: this.tempUnit,
            t: temps,
        };
        //console.log(ret);
        return ret;
    },
    loadProfile: function (a) {
        this.sd = new Date(a.s);
        this.tempUnit = a.u;
        this.clear();
        this.renderRows(a.t);
        profileChart.update(this.chartdata(), this.tempUnit);
    },
    initProfile: function (p) {
        if (typeof p != "undefined") {
            // start date
            var sd = new Date(p.s);
            this.tempUnit = p.u;
            profileEditor.initable(p.t, sd);
        } else {
            profileEditor.initable([], new Date());
        }
    },

    /** @param {TempUnit} u */
    setTempUnit: function (u) {
        if (u == this.tempUnit) return;
        this.tempUnit = u;
        var rl = this.getRows();
        for (let i = 0; i < rl.length; i++) {
            var tcell = rl[i].querySelector("td.stage-temp");
            var temp = parseFloat(tcell.innerHTML);
            if (!isNaN(temp))
                tcell.innerHTML = u == "C" ? F2C(temp) : C2F(temp);
        }
        profileChart.update(this.chartdata(), this.tempUnit);
    },
};

/* end of profile.js */
/* PL: profle list */
export var PL = {
    pl_path: "P",
    url_list: "/list",
    url_save: "/fputs",
    url_del: "/rm",
    url_load: "pl.php?ld=",
    div: "#profile-list-pane",
    shown: false,
    initialized: false,
    plist: [],
    path: function (a) {
        return "/" + this.pl_path + "/" + a;
    },
    rm: async function (e) {
        const url = `${this.url_del}?path=${this.path(this.plist[e])}`;
        try {
            await del(url);
            this.plist.splice(e, 1);
            this.list();
        } catch (error) {
            alert(error);
        }
    },
    load: async function (e) {
        const c = this.path(this.plist[e]);
        try {
            const json = await get(c);
            const a = JSON.parse(json);
            profileEditor.loadProfile(a);
        } catch (error) {
            console.warning(`Failed to load profile: ${error}`);
        }
    },
    list: function () {
        var a = this;
        var h = select(a.div).querySelector(".profile-list");
        var lis = h.querySelectorAll("li");
        for (let i = 0; i < lis.length; i++) {
            h.removeChild(lis[i]);
        }
        var b = a.row;
        a.plist.forEach(function (f, g) {
            var c = b.cloneNode(true);
            c.querySelector(".profile-name").innerHTML = f;
            c.querySelector(".profile-name").onclick = function (j) {
                j.preventDefault();
                a.load(g);
                return false;
            };
            c.querySelector(".rmbutton").onclick = function () {
                a.rm(g);
            };
            h.appendChild(c);
        });
    },
    append: function (b) {
        if (!this.initialized) {
            return;
        }
        this.plist.push(b);
        this.list();
    },
    init: async function () {
        this.initialized = true;
        this.row = select(this.div).querySelector("li");
        this.row.parentNode.removeChild(this.row);

        const url = `${this.url_list}?dir=${this.path("")}`;
        let data;
        try {
            const json = await get(url);
            data = JSON.parse(json);
        } catch (error) {
            alert(error);
            return;
        }

        this.plist = [];
        data.forEach((e) => {
            if (e.type == "file") {
                this.plist.push(e.name);
            }
        });
        this.list();
    },
    toggle: function () {
        if (!this.initialized) {
            this.init();
        }
        this.shown = !this.shown;
        if (this.shown) {
            select(this.div).style.display = "block";
        } else {
            select(this.div).style.display = "none";
        }
    },
    saveas: function () {
        select("#dlg_saveas").style.display = "block";
    },
    cancelSave: function () {
        select("#dlg_saveas").style.display = "none";
    },
    doSave: async function () {
        var e = select("#dlg_saveas input").value;
        if (e == "") {
            return;
        }
        if (e.match(/[\W]/g)) {
            return;
        }
        const g = profileEditor.getProfile();
        if (g === false) {
            alert("<%= script_control_invalid_value_check_again %>");
            return;
        }

        const url = `${this.url_save}?path=${this.path(e)}`;
        const payload = `content=${encodeURIComponent(JSON.stringify(g))}`;
        try {
            await post(url, payload, "form");
            this.append(e);
            this.cancelSave();
        } catch (error) {
            alert(error);
        }
    },
};
/* end of PL*/
var BrewPiSetting = {
    valid: false,
    maxDegree: 30,
    minDegree: 0,
    tempUnit: "C",
};

var modekeeper = {
    initiated: false,
    /** @type TemperatureControlMode[] */
    modes: ["profile", "beer", "fridge", "off"],
    /** @type TemperatureControlMode */
    cmode: "off",

    /** @param {TemperatureControlMode} m */
    dselect: function (m) {
        var d = byId(m + "-m");
        var nc = byId(m + "-m").className.replace(/\snav-selected/, "");
        d.className = nc;

        byId(m + "-s").style.display = "none";
    },

    /** @param {TemperatureControlMode} m */
    select: function (m) {
        byId(m + "-m").className += " nav-selected";
        byId(m + "-s").style.display = "block";
    },
    init: function () {
        var me = this;
        if (me.initiated) return;
        me.initiated = true;
        for (const mode of me.modes) {
            byId(mode + "-s").style.display = "none";
            byId(mode + "-m").onclick = function () {
                var tm = this.id.replace(/-m/, "");
                me.dselect(me.cmode);
                me.select(tm);
                me.cmode = tm;
                return false;
            };
        }
        me.cmode = "profile";
        me.select(me.cmode);
    },
    apply: function () {
        if (!BrewPiSetting.valid) {
            alert("<%= script_control_not_conected_to_controller %>");
            //		return;
        }
        if (this.cmode == "beer" || this.cmode == "fridge") {
            var v = byId(this.cmode + "-t").value;
            if (
                v == "" ||
                isNaN(v) ||
                v > BrewPiSetting.maxDegree ||
                v < BrewPiSetting.minDegree
            ) {
                alert("<%= script_control_invalid_temperature %>" + v);
                return;
            }
            if (this.cmode == "beer") {
                //console.log("j{mode:b, beerSet:" + v+ "}");
                BWF.send("j{mode:b, beerSet:" + v + "}");
            } else {
                console.log("j{mode:f, fridgeSet:" + v + "}");
                BWF.send("j{mode:f, fridgeSet:" + v + "}");
            }
        } else if (this.cmode == "off") {
            //console.log("j{mode:o}");
            BWF.send("j{mode:o}");
        } else {
            // should save first.
            if (profileEditor.dirty) {
                alert("<%= script_control_save_profile_before_applay %>");
                return;
            }
            //console.log("j{mode:p}");
            byId("dlg_beerprofilereminder").style.display = "block";
            byId("dlg_beerprofilereminder").querySelector("button.ok").onclick =
                async function () {
                    byId("dlg_beerprofilereminder").style.display = "none";
                    var gravity = parseFloat(
                        select("#dlg_beerprofilereminder input").value,
                    );
                    if (typeof updateOriginGravity == "function")
                        updateOriginGravity(gravity);
                    var data = {
                        name: "webjs",
                        og: 1,
                        gravity: gravity,
                    };
                    try {
                        await post("gravity", JSON.stringify(data), "json");
                        BWF.send("j{mode:p}");
                    } catch (error) {
                        alert(`<%= failed %>: ${error}`);
                    }
                };
            byId("dlg_beerprofilereminder").querySelector(
                "button.oknog",
            ).onclick = function () {
                byId("dlg_beerprofilereminder").style.display = "none";
                BWF.send("j{mode:p}");
            };
            byId("dlg_beerprofilereminder").querySelector(
                "button.cancel",
            ).onclick = function () {
                byId("dlg_beerprofilereminder").style.display = "none";
            };
        }
    },
};

export async function saveprofile() {
    //console.log("save");
    var r = profileEditor.getProfile();
    if (r === false) {
        alert("<%= script_control_invalid_value_check_again %>");
        return;
    }
    const json = JSON.stringify(r);
    console.log("result=" + json);

    try {
        await post(BPURL, `data=${encodeURIComponent(json)}`, "form");
        profileEditor.markdirty(false);
        alert("<%= done %>");
    } catch (error) {
        alert(`<%= script_control_failed_to_save %>: ${error}`);
    }
}

function updateTempUnit(u) {
    var Us = document.getElementsByClassName("t_unit");
    for (var i = 0; i < Us.length; i++) {
        Us[i].innerHTML = u;
    }
}

function ccparameter(s) {
    var setting = {
        valid: true,
        minDegree: s.tempSetMin,
        maxDegree: s.tempSetMax,
        tempUnit: s.tempFormat,
    };
    if (setting.tempUnit != BrewPiSetting.tempUnit) {
        updateTempUnit(setting.tempUnit);
        profileEditor.setTempUnit(setting.tempUnit);
    }
    BrewPiSetting = setting;
}

function rcvBeerProfile(p) {
    closeDlgLoading();
    updateTempUnit(p.u); // using profile temp before we get from controller
    BrewPiSetting.tempUnit = p.u;
    profileEditor.initProfile(p);
    profileChart = new ProfileChart("tc_chart", profileEditor.chartdata(), p.u);
}

export function initctrl() {
    updateNavbarVersion();
    Capper.init();
    modekeeper.init();
    let ptc = new PTC(select("#ptc-control"));
    openDlgLoading();

    BWF.init({
        onconnect: function () {
            BWF.send("c");
        },
        error: function () {
            //console.log("error");
            closeDlgLoading();
            communicationError();
        },
        handlers: {
            A: function (c) {
                if (typeof c["nn"] != "undefined") {
                    select("#hostname").innerHTML = c["nn"];
                }
                if (typeof c["ver"] != "undefined") {
                    if (JSVERSION != c["ver"])
                        alert("<%= script_control_version_mismatched %>");
                }
                if (typeof c["cap"] != "undefined") Capper.status(c["cap"]);
                if (typeof c["ptc"] != "undefined") ptc.config(c.ptc);
            },
            C: function (c) {
                ccparameter(c);
            },
            B: function (c) {
                rcvBeerProfile(c);
            },
        },
    });
}

window.modekeeper = modekeeper;
window.profileEditor = profileEditor;
window.saveprofile = saveprofile;
window.PL = PL;
