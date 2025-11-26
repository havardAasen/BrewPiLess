import { byId, select, s_ajax, updateNavbarVersion } from './shared';
import { BWF } from "./vendor/bwf";

function formatIP(ip) {
    return ip === "0.0.0.0" ? "" : ip;
}

function updateInput(input, value) {
    if (input.classList.contains("iptype")) {
        input.value = formatIP(value);
    } else if (input.type === "checkbox") {
        input.checked = value;
    } else {
        input.value = value;
    }
}

function loadSetting() {
    s_ajax({
        url: "config?cfg=1",
        m: "GET",
        success: function(data) {
            const json = JSON.parse(data);
            window.oridata = json;

            Object.entries(json).forEach(([key, value]) => {
                let input = select(`input[id=${key}]`);
                let wifiMode = select(`select[id=${key}]`);

                if (input) {
                    updateInput(input, value);
                } else if (wifiMode) {
                    wifiMode.value = value;
                }
            });
        },
        fail: function(d) {
            alert("<%= script_config_error_getting_data %>:" + d);
        }
    });
}

function waitrestart() {
    select("#waitprompt").style.display = "block";
    select("#inputform").style.display = "none";
    setTimeout(function() {
        window.location.reload();
    }, 15000);
}

export function saveSystemSettings() {
    const inputs = document.querySelectorAll("#sysconfig input, #sysconfig select");
    let json = {};
    let reboot = false;
    inputs.forEach(input => {
        if (!input.id) return;

        let val;
        if (input.type === "checkbox") {
            val = input.checked;
        } else if (input.type === "number" || input.tagName === "SELECT") {
            val = Number(input.value);
        } else {
            val = input.value.trim();
        }
        json[input.id] = val;

        if (window.oridata?.[input.id] !== val && !input.classList.contains("nb")) {
            reboot = true;
        }
    });

    var url = "config" + (reboot ? "" : "?nb");
    s_ajax({
        url: url,
        m: "POST",
        data: "data=" + encodeURIComponent(JSON.stringify(json)),
        success: function() {
            if (reboot) waitrestart();
        },
        fail: function(d) {
            alert("<%= script_config_error_saving_data %>:" + d);
        }
    });
}

export function load() {
    updateNavbarVersion();
    loadSetting();
    Net.init();
}


function validIP(ip) {
    const parts = ip.split(".");
    if (parts.length !== 4) return false;

    let value = 0;
    for (const part of parts) {
        if (!/^\d+$/.test(part)) return false; // Reject non-numeric or empty strings
        const num = Number(part);
        if (num < 0 || num > 255) return false;
        value = (value << 8) + num;
    }

    return value;
}

export var Net = {
    select: function(l) {
        byId('ssid').value = l.innerText || l.textContent;
        byId('nwpass').focus();
        return false;
    },
    init: function() {
        this.litem = select(".nwlist");
        this.litem.parentNode.removeChild(this.litem);
        this.setupEvent();
        this.hide();
    },
    nwevent: function(data) {
        var me = this;
        //console.log("ws:" + data);
        if (typeof data["list"] != "undefined") {
            me.list(data.list);
        } else if (typeof data["ssid"] != "undefined") {
            if (data.ssid != "") {
                select("#connected-ssid").innerHTML = data.ssid;
            }
            if (typeof data["ip"] != "undefined")
                if (data.ip != "") {
                    select("#sta-ip").innerHTML = data.ip;

                }
        }
    },
    rssi: function(x) {
        return (x > 0) ? "?" : Math.min(Math.max(2 * (x + 100), 0), 100);
    },
    list: function(nwlist) {
        var nws = select("#networks");
        nws.innerHTML = "";
        for (var i = 0; i < nwlist.length; i++) {
            var nl = this.litem.cloneNode(true);
            nl.getElementsByTagName("a")[0].innerHTML = nwlist[i].ssid;
            var signal = nl.getElementsByTagName("span")[0];
            signal.innerHTML = "" + this.rssi(nwlist[i].rssi) + "%";
            if (nwlist[i].enc) signal.className = signal.className + " l";
            nws.appendChild(nl);
        }
    },
    setupEvent: function() {
        var b = this;

        BWF.init({
            handlers: {
                W: function(ev) {
                    b.nwevent(ev);
                }
            }
        });
    },
    scan: function() {
        var lists = select("#networks");
        lists.innerHTML = "Scanning...";

        s_ajax({
            m: "GET",
            url: "/wifiscan",
            success: function() {}
        });
        return false;
    },
    save: function() {
        var data = "nw=" + encodeURIComponent(byId("ssid").value);
        if (byId("nwpass").value != "") data = data + "&pass=" + encodeURIComponent(byId("nwpass").value);
        var ip = validIP(byId("ip").value);
        var gw = validIP(byId("gw").value);
        var nm = validIP(byId("mask").value);
        var dns = validIP(byId("dns").value);
        if (ip && gw && nm) {
            data = data + "&ip=" + byId("ip").value.trim() +
                "&gw=" + byId("gw").value.trim() +
                "&nm=" + byId("mask").value.trim() +
                "&dns=" + byId("dns").value.trim();
        }
        s_ajax({
            m: "POST",
            url: "/wificon",
            data: data,
            success: function() {}
        });
        this.hide();
        return false;
    },
    show: function() {
        select("#networkselection").style.display = "block";
    },
    hide: function() {
        select("#networkselection").style.display = "none";
    }
};

window.Net = Net
window.saveSystemSettings = saveSystemSettings
