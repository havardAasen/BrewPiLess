import { select } from "./shared";
import { get, post } from "./httpClient";

export async function mqttLoadSetting() {
    let json;
    try {
        json = await get("mqtt");
    } catch (error) {
        alert(`<%= script_config_error_getting_data %>: ${error}`);
        return;
    }

    const data = JSON.parse(json);
    Object.keys(data).map(function (key) {
        var div = select(`.mqtt-input[name=mqtt_${key}]`);
        if (div) {
            if (div.type == "checkbox") div.checked = data[key] != 0;
            else div.value = data[key];
        }
    });
}

async function mqttSave() {
    var ins = document.querySelectorAll(".mqtt-input");
    var json = {};
    Object.keys(ins).map(function (key, i) {
        if (ins[i].name && ins[i].name != "") {
            var val;
            if (ins[i].type === "checkbox") val = ins[i].checked;
            else if (ins[i].type === "number") val = Number(ins[i].value);
            else val = ins[i].value.trim();
            json[ins[i].name.split("_")[1]] = val;
        }
    });

    console.log(JSON.stringify(json));
    const payload = `data=${encodeURIComponent(JSON.stringify(json))}`;
    try {
        await post("mqtt", payload, "form");
        alert("<%= done %>!");
    } catch (error) {
        alert(`<%= script_config_error_saving_data %>: ${error}`);
    }
}

window.mqttSave = mqttSave;
