/**
 * Performs an HTTP GET request.
 *
 * @param url - The absolute or relative URL to request.
 * @returns A promise that resolves with the response text
 *          when the request succeeds.
 *
 * @throws {Error} If the request fails or the server responds with a non‑OK
 *                 HTTP status code.
 */
export async function get(url: string): Promise<string> {
    const res = await fetch(url, {
        method: "GET",
    });

    if (!res.ok) {
        throw new Error(`GET ${url} failed with status ${res.status}`);
    }

    return res.text();
}

/**
 * Performs an HTTP DELETE request.
 *
 * @param url - The absolute or relative URL to request.
 * @returns A promise that resolves with the response text
 *          when the request succeeds.
 *
 * @throws {Error} If the request fails or the server responds with a non‑OK
 *                 HTTP status code.
 */
export async function del(url: string): Promise<string> {
    const res = await fetch(url, { method: "DELETE" });

    if (!res.ok) {
        throw new Error(`DELETE ${url} failed with status ${res.status}`);
    }

    return res.text();
}

/**
 * Sends an HTTP POST request.
 *
 * @param url - The absolute or relative URL to request.
 * @param body - The request payload, either a raw string or an object.
 * @param contentType - How the body should be encoded.
 * @returns A promise that resolves with the response text
 *          when the request succeeds.
 *
 * @throws {Error} If the request fails or the server responds with a non‑OK
 *                 HTTP status code.
 */
export async function post(
    url: string,
    body: string | Record<string, unknown>,
    contentType: "form" | "json" = "form",
): Promise<string> {
    let payload: BodyInit;
    let headers: HeadersInit;

    if (contentType === "json") {
        payload = JSON.stringify(body);
        headers = { "Content-Type": "application/json" };
    } else {
        payload =
            typeof body === "string"
                ? body
                : new URLSearchParams(
                      Object.entries(body).map(([k, v]) => [k, String(v)]),
                  ).toString();

        headers = { "Content-Type": "application/x-www-form-urlencoded" };
    }

    const res = await fetch(url, {
        method: "POST",
        headers,
        body: payload,
    });

    if (!res.ok) {
        throw new Error(`POST ${url} failed with status ${res.status}`);
    }

    return res.text();
}
