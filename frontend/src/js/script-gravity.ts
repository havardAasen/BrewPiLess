import { select, s_ajax, updateNavbarVersion } from "./shared";

const gdcurl = "/gdc";

function fill(setting: Record<string, any>): void {
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
    const setting: Record<string, any> = {};
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
        fail: function (a: any) {
            alert("<%= script_control_failed_updating_data %>" + a);
        },
    });
}

export function init(): void {
    updateNavbarVersion();

    s_ajax({
        url: gdcurl + "?data",
        m: "GET",
        success: function (a: string) {
            fill(JSON.parse(a));
        },
        fail: function () {
            //alert("failed getting data:" + a)
        },
    });
}

(window as any).save = save;
