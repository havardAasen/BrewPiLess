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

export const BWF = {
    BrewProfile: "/brewing.json",

    ws: null as WebSocket | null,
    handlers: {} as BWFHandlers,
    raw: null as ((msg: string) => void) | null,
    onconnect: (() => {}) as () => void,
    error: (code: number) => {},
    auto: true,
    reconnecting: false,
    gotMsg: false,
    rcCount: 0,

    process(msg: string) {
        if (this.raw != null) {
            this.raw(msg);
            return;
        }

        let m: Record<string, any> = {};
        eval("m={" + msg + "}");

        for (const key in m) {
            if (typeof this.handlers[key] !== "undefined") {
                this.handlers[key](m[key]);
            }
        }
    },

    on(label: string, handler: (value: any) => void) {
        this.handlers[label] = handler;
    },

    send(data: string) {
        if (this.ws && this.ws.readyState === WebSocket.OPEN) {
            this.ws.send(data);
        }
    },

    connect() {
        const me = this;

        if (typeof WebSocket !== "undefined") {
            const ws = new WebSocket("ws://" + document.location.host + "/ws");
            me.ws = ws;

            ws.onopen = function () {
                console.log("Connected");
                me.onconnect();
            };

            ws.onclose = function () {
                if (me.reconnecting) return;
                console.log("WS close");
                me.error(-2);

                if (me.auto) {
                    setTimeout(function () {
                        me.reconnect();
                    }, 5000);
                }
            };

            ws.onmessage = function (e: MessageEvent) {
                me.process(e.data);
            };
        } else {
            alert("Error! WebSocket Not Supported!");
        }
    },

    reconnect(forced?: boolean) {
        forced = typeof forced === "undefined" ? false : true;
        const me = this;

        if (me.reconnecting) return;
        if (!forced && me.ws && me.ws.readyState === WebSocket.OPEN) return;

        console.log(
            "reconnect forced:" + forced + " state:" + me.ws?.readyState,
        );

        me.reconnecting = true;
        me.ws?.close();
        me.connect();
        me.reconnecting = false;
    },

    init(arg: BWFInitOptions) {
        const b = this;

        b.error = arg.error ?? (() => {});
        b.handlers = arg.handlers ?? {};
        b.raw = arg.raw ?? null;
        b.onconnect = arg.onconnect ?? (() => {});
        b.auto = arg.reconnect ?? true;

        b.connect();
    },

    save(
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
    },

    load(
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
    },
};
