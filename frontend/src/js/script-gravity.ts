import { select, s_ajax, updateNavbarVersion } from "./shared";
import { get } from "./httpClient";

const gdcurl = "/gdc";

function fill(setting: Record<string, boolean | string>): void {
    for (const name in setting) {
        const element = select(
            `input[name=${name}]`,
        ) as HTMLInputElement | null;
        if (element) {
            if (element.type === "checkbox") {
                element.checked = Boolean(setting[name]);
            } else {
                element.value = String(setting[name]);
            }
        }
    }
}

function save(): void {
    const inputs = document.getElementsByTagName("input");
    const setting: Record<string, boolean | string> = {};
    Array.from(inputs).forEach((ele) => {
        if (!ele.name) return;
        if (ele.type === "checkbox") {
            setting[ele.name] = ele.checked;
        } else if (ele.type === "text") {
            setting[ele.name] = ele.value;
        }
    });
    //    console.log("result=" + setting);

    s_ajax({
        url: gdcurl,
        m: "POST",
        mime: "application/json",
        data: JSON.stringify(setting),
        success: function () {
            alert("<%= done %>");
        },
        fail: function (a: ProgressEvent | number) {
            alert("<%= script_control_failed_updating_data %>" + a);
        },
    });
}

export async function init(): Promise<void> {
    updateNavbarVersion();

    try {
        const json = await get(`${gdcurl}?data`);
        fill(JSON.parse(json));
    } catch (error) {
        console.warn(error);
    }
}

window.Save = save;
