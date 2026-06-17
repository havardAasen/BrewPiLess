type JSONValue = string | number | boolean | null | JSONObject | JSONValue[];

interface JSONObject {
    [key: string]: JSONValue;
}

function parseMessage(msg: string): Record<string, string> {
    const idx = msg.indexOf(":");
    if (idx === -1) throw new Error("Invalid message: no colon");

    const key = msg.slice(0, idx);
    const value = msg.slice(idx + 1);

    return { [key]: JSON.parse(value) };
}

interface BWFHandlers {
    [key: string]: (value: string) => void;
}

interface BWFInitOptions {
    handlers: BWFHandlers;
    raw?: ((msg: string) => void) | null;
    reconnect?: boolean;
    onconnect?: () => void;
    onclose?: () => void;
    onNoMessage?: () => void;
    error?: () => void;
}

const enum WSState {
    Idle,
    Connecting,
    Connected,
    Reconnecting,
    Closed,
}

class BWFClient {
    private ws: WebSocket | null = null;
    private state: WSState = WSState.Idle;

    private reconnectTimer: number | null = null;
    private watchdogTimer: number | null = null;

    private retryCount = 0;
    private readonly maxRetryDelay = 30000; // 30s cap

    private gotMsg = false;
    private raw: ((msg: string) => void) | null = null;

    public onconnect: (() => void) | null = null;
    public onclose: (() => void) | null = null;
    public onNoMessage: (() => void) | null = null;
    public error: (() => void) | null = null;
    public handlers: BWFHandlers = {};

    public on(label: string, handler: (value: JSONValue) => void) {
        this.handlers[label] = handler;
    }

    public send(data: string) {
        if (this.ws && this.ws.readyState === WebSocket.OPEN) {
            this.ws.send(data);
        }
    }

    public init(arg: BWFInitOptions) {
        this.onconnect = arg.onconnect ?? (() => {});
        this.onclose = arg.onclose ?? (() => {});
        this.onNoMessage = arg.onNoMessage ?? (() => {});
        this.error = arg.error ?? (() => {});
        this.handlers = arg.handlers;
        this.raw = arg.raw ?? null;

        this.connect();
    }

    private connect() {
        if (
            this.state === WSState.Connecting ||
            this.state === WSState.Connected
        ) {
            return;
        }
        try {
            this.ws = new WebSocket("ws://" + document.location.host + "/ws");
        } catch (err) {
            console.error("WebSocket creation failed", err);
            return;
        }

        this.ws.onopen = () => {
            console.log("WS connected");
            this.state = WSState.Connected;
            this.retryCount = 0;
            this.clearReconnectTimer();
            this.startWatchdog();
            if (this.onconnect) {
                this.onconnect();
            }
        };

        this.ws.onclose = () => {
            console.log("WS close");
            this.state = WSState.Reconnecting;
            if (this.onclose) {
                this.onclose();
            }
            this.scheduleReconnect();
        };

        this.ws.onerror = () => {
            if (this.error) {
                this.error();
            }
        };

        this.ws.onmessage = (e: MessageEvent) => {
            this.gotMsg = true;
            this.process(e.data);
        };
    }

    private scheduleReconnect() {
        if (this.reconnectTimer !== null) return;

        const delay = Math.min(this.maxRetryDelay, 1000 * 2 ** this.retryCount);
        this.retryCount++;

        this.reconnectTimer = window.setTimeout(() => {
            this.reconnectTimer = null;
            this.connect();
        }, delay);
    }

    private clearReconnectTimer() {
        if (this.reconnectTimer !== null) {
            clearTimeout(this.reconnectTimer);
            this.reconnectTimer = null;
        }
    }

    private startWatchdog() {
        if (this.watchdogTimer) {
            clearInterval(this.watchdogTimer);
        }

        this.watchdogTimer = window.setInterval(() => {
            if (!this.gotMsg) {
                this.onNoMessage?.();
                this.scheduleReconnect();
                return;
            }

            this.gotMsg = false;
        }, 10000);
    }

    private process(msg: string) {
        if (this.raw != null) {
            this.raw(msg);
            return;
        }

        const json: Record<string, string> = parseMessage(msg);

        for (const key in json) {
            if (typeof this.handlers[key] !== "undefined") {
                this.handlers[key](json[key]);
            }
        }
    }
}

export const BWF = new BWFClient();
