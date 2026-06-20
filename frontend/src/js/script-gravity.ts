import { byId, select, updateNavbarVersion } from "./shared";
import { get, post } from "./httpClient";

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

async function save(): Promise<void> {
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

    try {
        await post(gdcurl, JSON.stringify(setting), "json");
        alert("<%= done %>");
    } catch (error) {
        alert(`<%= script_control_failed_updating_data %>: ${error}`);
    }
}

export async function init(): Promise<void> {
    updateNavbarVersion();

    const form = byId<HTMLFormElement>("gravity-form")!;
    form.addEventListener("submit", (ev) => {
        ev.preventDefault();
        save();
    });

    try {
        const json = await get(`${gdcurl}?data`);
        fill(JSON.parse(json));
    } catch (error) {
        console.warn(error);
    }
}
