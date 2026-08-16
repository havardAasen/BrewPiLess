import { byId } from "./shared";
import { BWF } from "./bwf";

/**
 * @brief Escape characters that have special meaning in HTML.
 *
 * Replaces &, <, >, " and ' with their entity equivalents so the text can be
 * safely inserted into `innerHTML`.
 *
 * @param unsafe – The raw string that may contain HTML‑special characters.
 * @returns The escaped string.
 */
function escapeHtml(unsafe: string): string {
    return unsafe
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;")
        .replace(/"/g, "&quot;")
        .replace(/'/g, "&#039;");
}

/**
 * Append a timestamped entry to the log
 *
 * @param direction - 'D' for download, 'U' for upload.
 * @param message - text to display in the log
 */
function log(direction: "D" | "U", message: string): void {
    const logEl = byId("log")!;
    const now = new Date();
    const time = now.toTimeString().split(" ")[0];
    const arrow = direction === "D" ? "&darr;" : "&uarr;";

    const line = `${time} ${arrow} ${escapeHtml(message)}<br>`;
    logEl.insertAdjacentHTML("beforeend", line);
}

function sendCmd() {
    const cmdInput = byId<HTMLButtonElement>("command")!;
    const data = cmdInput.value.trim();
    if (!data) return;

    BWF.send(data);
    log("U", data);

    cmdInput.value = "";
}

function clearLogs(): void {
    byId("log")!.innerHTML = "";
}

export function init() {
    byId("clearLog")!.addEventListener("click", clearLogs);
    byId("sendCmd")!.addEventListener("click", sendCmd);
    byId("command")!.addEventListener("keydown", (ev) => {
        if (ev.key === "Enter") {
            sendCmd();
        }
    });

    BWF.init({
        handlers: {
            fakeHandler: () => {},
        },
        error: () => console.error("WebSocket error"),
        raw: (message: string) => log("D", message),
    });
}
