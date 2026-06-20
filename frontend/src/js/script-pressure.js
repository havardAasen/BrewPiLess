import { get, post } from "./httpClient";
import { byId, select, updateNavbarVersion } from "./shared";

var PCTRL = {
    init: async function () {
        let json;
        try {
            json = await get("psi");
        } catch (error) {
            alert(error);
            return;
        }

        const data = JSON.parse(json);
        if (data.mode == 1) {
            select("#pt-enabled").checked = true;
            select("#pt-control").checked = false;
        } else if (data.mode == 2) {
            select("#pt-enabled").checked = true;
            select("#pt-control").checked = true;
        } else {
            select("#pt-enabled").checked = false;
            select("#pt-control").checked = false;
        }
        select("#fpb").value = data.b;
        select("#fpa").value = data.a;
    },
    apply: async function () {
        // save data to BPL
        var data = {};
        if (select("#pt-enabled").checked) {
            if (select("#pt-control").checked) data.mode = 2;
            else data.mode = 1;
        } else data.mode = 0;
        data.a = parseFloat(select("#fpa").value);
        data.b = parseFloat(select("#fpb").value);

        const payload = `data=${encodeURIComponent(JSON.stringify(data))}`;
        try {
            await post("psi", payload, "form");
            alert("<%= done %>");
        } catch (error) {
            alert(`<%= script_control_failed_to_save %>: f${error}`);
        }
    },
    cal: function () {
        select("#dlg_calibrate").style.display = "block";
        //        Q("#cal1").disabled = true;
    },
    xcal: function () {
        select("#dlg_calibrate").style.display = "none";
    },
    cal0: async function () {
        try {
            const json = await get("psi?r=1");
            const data = JSON.parse(json);
            select("#fpb").value = data.a0;
        } catch (error) {
            alert(error);
        }
    },
    cal1: async function () {
        try {
            const json = await get("psi?r=1");
            const data = JSON.parse(json);
            select("#fpa").value = PCTRL.conv(data.a0);
        } catch (error) {
            alert(error);
        }
    },
    conv: function (a0) {
        var b = parseFloat(select("#fpb").value);
        var psi = parseFloat(select("#calpsi").value);
        //(a0 - b) * a =  psi
        // a = psi / (a0-b)
        return (psi / (a0 - b)).toFixed(4);
    },
};

export function loaded() {
    updateNavbarVersion();
    PCTRL.init();

    const form = byId("transducer-control");
    form.addEventListener("submit", (ev) => {
        ev.preventDefault();
        PCTRL.apply();
    });

    const calibrate = byId("calibrate-btn");
    calibrate.addEventListener("click", () => {
        PCTRL.cal();
    });
}
