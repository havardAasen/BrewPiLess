import { byId, select, s_ajax, updateNavbarVersion } from './shared';
import { BWF } from "./vendor/bwf";

function formatIP(ip: string): string {
    return ip === "0.0.0.0" ? "" : ip;
}

function updateInput(input: HTMLInputElement, value: string | boolean): void {
    if (input.classList.contains("iptype")) {
        input.value = formatIP(String(value));
    } else if (input.type === "checkbox") {
        input.checked = Boolean(value);
    } else {
        input.value = String(value);
    }
}

function loadSetting() {
    s_ajax({
        url: "config?cfg=1",
        m: "GET",
        success: (data: string) => {
            const json: Record<string, string> = JSON.parse(data);
            window.oridata = json;

            Object.entries(json).forEach(([key, value]) => {
                const input = select<HTMLInputElement>(`input[id=${key}]`);
                const wifiMode = select<HTMLInputElement>(`select[id=${key}]`);

                if (input) {
                    updateInput(input, value);
                } else if (wifiMode) {
                    wifiMode.value = value;
                }
            });
        },
        fail: (err) => {
            alert(`<%= script_config_error_getting_data %>: ${err}`);
        },
    });
}

function waitrestart() {
    select<HTMLDivElement>("#waitprompt")!.style.display = "block";
    select<HTMLDivElement>("#inputform")!.style.display = "none";

    setTimeout(() => {
        window.location.reload();
    }, 15000);
}

export function saveSystemSettings(): void {
    const inputs = document.querySelectorAll<HTMLInputElement | HTMLSelectElement>(
        "#sysconfig input, #sysconfig select"
    );

    const json: Record<string, string | number | boolean> = {};
    let reboot = false;

    inputs.forEach(input => {
        if (!input.id) return;

        let val: string | number | boolean;
        if (input instanceof HTMLInputElement && input.type === "checkbox") {
            val = input.checked;
        } else if (
            (input instanceof HTMLInputElement && input.type === "number") ||
            input instanceof HTMLSelectElement
        ) {
            val = Number(input.value);
        } else {
            val = input.value.trim();
        }
        json[input.id] = val;

        if (window.oridata?.[input.id] !== val && !input.classList.contains("nb")) {
            reboot = true;
        }
    });

    const url = "config" + (reboot ? "" : "?nb");
    s_ajax({
        url: url,
        m: "POST",
        data: "data=" + encodeURIComponent(JSON.stringify(json)),
        success: function() {
            if (reboot) waitrestart();
        },
        fail(err): void {
            alert(`%= script_config_error_saving_data %>: ${err}`);
        },
    });
}

export function load(): void {
    updateNavbarVersion();
    loadSetting();
    Net.init();
}


function validIP(ip: string): false | string {
    const parts = ip.split(".");
    if (parts.length !== 4) return false;

    const address: string[] = [];
    for (const part of parts) {
        if (!/^\d+$/.test(part)) return false; // Reject non-numeric or empty strings
        const num = Number(part);
        if (num < 0 || num > 255) return false;
        address.push(String(num));
    }

    return address.join(".");
}

interface NetworkEntry {
    ssid: string;
    rssi: number;
    enc?: boolean;
}

export const Net = {
    litem: null as HTMLElement | null,

    select(l: HTMLElement): boolean {
        byId<HTMLInputElement>('ssid')!.value = l.innerText || l.textContent || "";
        return false;
    },

    init(): void {
        this.litem = select(".nwlist")!;
        this.litem.parentNode!.removeChild(this.litem);
        this.setupEvent();
        this.hide();
    },

    nwevent(data: Record<string, any>) {
        if (typeof data["list"] != "undefined") {
            this.list(data.list);
        } else if (typeof data["ssid"] != "undefined") {
            if (data.ssid != "") {
                select<HTMLElement>("#connected-ssid")!.innerHTML = data.ssid;
            }
            if (typeof data["ip"] !== "undefined" && data.ip !== "") {
                select("#sta-ip")!.innerHTML = data.ip;
            }
        }
    },

    rssi(x: number): string | number {
        return x > 0 ? "?" : Math.min(Math.max(2 * (x + 100), 0), 100);
    },

    list(nwlist: NetworkEntry[]): void {
        const nws = select("#networks");
        if (!nws || !this.litem) return;

        nws.innerHTML = "";
        for (const nw of nwlist) {
            const nl = this.litem.cloneNode(true) as HTMLElement;
            const anchor = nl.getElementsByTagName("a")[0];
            if (anchor) anchor.innerHTML = nw.ssid;

            const signal = nl.getElementsByTagName("span")[0];
            if (signal) {
                signal.innerHTML = `${this.rssi(nw.rssi)}%`;
                if (nw.enc) signal.className = signal.className + " l";
            }
            nws.appendChild(nl);
        }
    },

    setupEvent(): void {
        BWF.init({
            handlers: {
                W: (ev: NetworkEntry) => this.nwevent(ev),
            },
        });
    },

    scan(): boolean {
        select<HTMLElement>("#networks")!.innerHTML = "Scanning...";

        s_ajax({
            m: "GET",
            url: "/wifiscan",
        });
        return false;
    },

    save(): boolean {
        let data = "nw=" + encodeURIComponent(byId<HTMLInputElement>("ssid")!.value);
        const pass = byId<HTMLInputElement>("nwpass")!.value;
        if (pass !== "") data += "&pass=" + encodeURIComponent(pass);

        const ip = validIP(byId<HTMLInputElement>("ip")!.value);
        const gw = validIP(byId<HTMLInputElement>("gw")!.value);
        const nm = validIP(byId<HTMLInputElement>("mask")!.value);
        const dns = validIP(byId<HTMLInputElement>("dns")!.value);

        if (ip && gw && nm) {
            data += `&ip=${ip}&gw=${gw}&nm=${nm}`;
            if (dns) data += `&dns=${dns}`;
        }

        s_ajax({
            m: "POST",
            url: "/wificon",
            data: data,
        });
        this.hide();
        return false;
    },
    show(): void {
        select<HTMLElement>("#networkselection")!.style.display = "block";
    },
    hide(): void {
        select<HTMLElement>("#networkselection")!.style.display = "none";
    },
};

window.Net = Net;
window.saveSystemSettings = saveSystemSettings;
