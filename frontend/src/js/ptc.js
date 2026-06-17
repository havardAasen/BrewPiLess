import { select } from "./shared";
import { post } from "./httpClient";

var saveurl = "/ptc";

export var PTC = {
    fill: function (setting) {
        for (var name in setting) {
            var ele = select("input[name=" + name + "]");
            if (ele) {
                ele.value = setting[name];
            }
        }
    },

    apply: async function () {
        var inputs = this.div.querySelectorAll("input");
        var setting = {};
        for (var i = 0; i < inputs.length; i++) {
            var ele = inputs[i];
            if (ele.name && ele.name != "") {
                setting[ele.name] = parseFloat(ele.value);
            }
        }

        const payload = `c=${encodeURIComponent(JSON.stringify(setting))}`;
        try {
            await post(saveurl, payload, "form");
            alert("<%= done %>!");
        } catch (error) {
            alert(error);
        }
    },

    config: function (a) {
        if (a.enabled) {
            this.div.style.display = "block";
            this.fill(a);
        }
    },
    init: function (div) {
        div.style.display = "none";
        this.div = div;
    },
};
