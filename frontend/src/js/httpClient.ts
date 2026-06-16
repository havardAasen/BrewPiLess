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
