import assert from "node:assert/strict";
import { describe, test } from "node:test";
import { gravityFilter } from "../src/js/chart/GravityFilter";

describe("GravityFilter", () => {
    test("resets clears internal state", () => {
        // Step 1: Add a value so y becomes non-zero
        gravityFilter.reset();
        gravityFilter.add(10); // y = 10
        gravityFilter.add(20); // y = 11

        // Sanity check: y should NOT be zero, or thirty before reset
        const beforeReset = gravityFilter.add(30); // y = 12.9
        assert.notEqual(beforeReset, [0, 30]);

        // Step 2: Reset
        gravityFilter.reset();

        // Step 3: After reset, the first add() should return the raw value
        const afterReset = gravityFilter.add(50);
        assert.equal(afterReset, 50);
    });

    test("applies smoothing", () => {
        gravityFilter.reset();
        gravityFilter.add(10);
        const result = gravityFilter.add(20);

        // Expected: y = 10 + 0.1 * (20 - 10) = 11
        assert.equal(result, 11);
    });

    test("setBeta changes smoothing factor", () => {
        gravityFilter.reset();
        gravityFilter.setBeta(0.5);

        gravityFilter.add(10);
        const result = gravityFilter.add(20);

        // Expected: y = 10 + 0.5 * (20 - 10) = 15
        assert.equal(result, 15);
    });
});
