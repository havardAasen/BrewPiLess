import { post } from "./httpClient";

export class PTC {
    private readonly url = "/ptc";

    constructor(private div: HTMLFormElement) {
        this.div.style.display = "none";

        this.div.addEventListener("submit", (ev) => {
            ev.preventDefault();
            this.apply();
        });
    }

    private fill(settings: Record<string, number | boolean>) {
        for (const [key, value] of Object.entries(settings)) {
            const input = this.div.querySelector<HTMLInputElement>(
                `input[name="${key}"]`,
            );
            if (input) input.value = String(value);
        }
    }

    private async apply() {
        const form = new FormData(this.div);
        const settings: Record<string, number> = {};

        form.forEach((value, key) => {
            settings[key] = parseFloat(value as string);
        });

        const payload = `c=${encodeURIComponent(JSON.stringify(settings))}`;
        try {
            await post(this.url, payload, "form");
            alert("<%= done %>!");
        } catch (error) {
            alert(error);
        }
    }

    config(settings: Record<string, number | boolean>) {
        this.div.style.display = settings.enabled ? "block" : "none";
        if (settings.enabled) this.fill(settings);
    }
}
