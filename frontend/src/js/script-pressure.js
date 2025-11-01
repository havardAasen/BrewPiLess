import { select, s_ajax, updateNavbarVersion } from "./shared";

var PCTRL = {
    init: function() {
        // get values from BPL
        s_ajax({
            url: "psi",
            m: "GET",
            success: function(json) {
                var d = JSON.parse(json);
                if (d.mode == 1) {
                    select("#pt-enabled").checked = true;
                    select("#pt-control").checked = false;
                } else if (d.mode == 2) {
                    select("#pt-enabled").checked = true;
                    select("#pt-control").checked = true;
                } else {
                    select("#pt-enabled").checked = false;
                    select("#pt-control").checked = false;
                }
                select("#fpb").value = d.b;
                select("#fpa").value = d.a;
            },
            fail: function(b) {
                alert("failed to connect to BPL.");
            }
        });

    },
    apply: function() {
        // save data to BPL
        var data = {};
        if (select("#pt-enabled").checked) {
            if (select("#pt-control").checked) data.mode = 2;
            else data.mode = 1;
        } else data.mode = 0;
        data.a = parseFloat(select("#fpa").value);
        data.b = parseFloat(select("#fpb").value);
        var json = JSON.stringify(data);
        s_ajax({
            url: "psi",
            m: "POST",
            mime: "application/x-www-form-urlencoded",
            data: "data=" + encodeURIComponent(json),
            success: function(d) {
                alert("<%= done %>")
            },
            fail: function(d) {
                alert("<%= script_control_failed_to_save %>");
            }
        });

    },
    cal: function() {
        select("#dlg_calibrate").style.display = "block";
        //        Q("#cal1").disabled = true;
    },
    xcal: function() {
        select("#dlg_calibrate").style.display = "none";
    },
    cal0: function() {
        s_ajax({
            url: "psi?r=1",
            m: "GET",
            success: function(json) {
                var d = JSON.parse(json);
                //                Q("#cal1").disabled = false;
                select("#fpb").value = d.a0;
            },
            fail: function() {
                alert("failed to connect to BPL.");
            }
        });
    },
    cal1: function() {
        s_ajax({
            url: "psi?r=1",
            m: "GET",
            success: function(json) {
                var d = JSON.parse(json);
                select("#fpa").value = PCTRL.conv(d.a0);
            },
            fail: function() {
                alert("failed to connect to BPL.");
            }
        });
    },
    conv: function(a0) {
        var b = parseFloat(select("#fpb").value);
        var psi = parseFloat(select("#calpsi").value);
        //(a0 - b) * a =  psi
        // a = psi / (a0-b)
        return (psi / (a0 - b)).toFixed(4);
    }
};

export function loaded() {
    updateNavbarVersion();
    PCTRL.init();
}
