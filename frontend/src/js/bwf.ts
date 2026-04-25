interface InvokeArgs {
    m: string; // HTTP method
    url: string;
    data?: string;
    mime?: string;
    success: (response: string) => void;
    fail?: (status: number) => void;
    timeout?: () => void;
}

function invoke(arg: InvokeArgs): void {
    const xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function () {
        if (xhttp.readyState === 4) {
            if (xhttp.status === 200) {
                arg.success(xhttp.responseText);
            } else {
                arg.fail?.(xhttp.status);
            }
        }
    };

    xhttp.ontimeout = function () {
        if (arg.timeout) arg.timeout();
        else arg.fail?.(-1);
    };

    xhttp.onerror = function (a: any) {
        if (arg.fail) arg.fail(typeof a === "number" ? a : -1);
    };

    xhttp.open(arg.m, arg.url, true);

    if (arg.data !== undefined) {
        xhttp.setRequestHeader(
            "Content-Type",
            arg.mime ?? "application/x-www-form-urlencoded",
        );
        xhttp.send(arg.data);
    } else {
        xhttp.send();
    }
}

type JSONValue = string | number | boolean | null | JSONObject | JSONValue[];

interface JSONObject {
    [key: string]: JSONValue;
}

function parseMessage(msg: string): Record<string, JSONValue> {
    const idx = msg.indexOf(":");
    if (idx === -1) throw new Error("Invalid message: no colon");

    const key = msg.slice(0, idx);
    const value = msg.slice(idx + 1);

    return { [key]: JSON.parse(value) };
}

interface BWFHandlers {
    [key: string]: (value: any) => void;
}

interface BWFInitOptions {
    handlers: BWFHandlers;
    raw?: ((msg: string) => void) | null;
    onconnect?: () => void;
    error?: (code: number) => void;
    reconnect?: boolean;
}

class BWFClient {
    private ws: WebSocket | null = null;
    private raw: ((msg: string) => void) | null = null;
    private auto = true;
    private reconnecting = false;

    public handlers: BWFHandlers = {};
    public onconnect: (() => void) | null = null;
    public error: ((code: number) => void) | null = null;
    public gotMsg = false;
    public rcCount = 0;


    public process(msg: string) {
        if (this.raw != null) {
            this.raw(msg);
            return;
        }

        const json: Record<string, JSONValue> = parseMessage(msg);

        for (const key in json) {
            if (typeof this.handlers[key] !== "undefined") {
                this.handlers[key](json[key]);
            }
        }
    }

    public on(label: string, handler: (value: any) => void) {
        this.handlers[label] = handler;
    }

    public send(data: string) {
        if (this.ws && this.ws.readyState === WebSocket.OPEN) {
            this.ws.send(data);
        }
    }

    public connect() {
        try {
            this.ws = new WebSocket("ws://" + document.location.host + "/ws");
        } catch (err) {
            console.error("WebSocket creation failed", err);
           return;
        }

        this.ws.onopen = () => {
            console.log("Connected");
            if (this.onconnect) {
                this.onconnect();
            }
        };

        this.ws.onclose = () => {
            if (this.reconnecting) return;
            console.log("WS close");
            if (this.error) {
                this.error(-2);
            }

            if (this.auto) {
                setTimeout(() => {
                    this.reconnect();
                }, 5000);
            }
        };

        this.ws.onmessage = (e: MessageEvent) => {
            this.process(e.data);
        };
    }

    public reconnect(forced?: boolean) {
        forced = forced ?? false;

        if (this.reconnecting) return;
        if (!forced && this.ws && this.ws.readyState === WebSocket.OPEN) return;

        console.log(
            "reconnect forced:" + forced + " state:" + this.ws?.readyState,
        );

        this.reconnecting = true;
        this.ws?.close();
        this.connect();
        this.reconnecting = false;
    }

    public init(arg: BWFInitOptions) {
        this.error = arg.error ?? (() => {});
        this.handlers = arg.handlers;
        this.raw = arg.raw ?? null;
        this.onconnect = arg.onconnect ?? (() => {});
        this.auto = arg.reconnect ?? true;

        this.connect();
    }

    public save(
        file: string,
        data: string,
        success: () => void,
        fail: (e: ProgressEvent | number) => void,
    ) {
        invoke({
            m: "POST",
            url: "/fputs",
            data: "path=" + file + "&content=" + encodeURIComponent(data),
            success: function () {
                success();
            },
            fail: function (e: ProgressEvent | number) {
                fail(e);
            },
        });
    }

    public load(
        file: string,
        success: (d: string) => void,
        fail: (e: ProgressEvent | number) => void,
    ) {
        invoke({
            m: "GET",
            url: file,
            success: function (d: string) {
                success(d);
            },
            fail: function (e: ProgressEvent | number) {
                fail(e);
            },
        });
    }
}

export const BWF = new BWFClient();
