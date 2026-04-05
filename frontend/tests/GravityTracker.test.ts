import assert from "node:assert/strict";
import { describe, test } from "node:test";
import { gravityTracker } from "../src/js/chart/GravityTracker";

describe("GravityTracker", () => {
    test("addRecord stores values circularly", () => {
        gravityTracker.init();

        const slots = gravityTracker.NumberOfSlots;
        for (let i = 0; i < slots + 5; i++) {
            gravityTracker.addRecord(i);
        }

        // After overflow, record[4] should contain the last written value
        const expected = slots + 4;
        assert.equal(gravityTracker["record"][4], expected);
    });

    test("stable detects stable values", () => {
        gravityTracker.init();

        // Fill with stable values
        for (let i = 0; i < 48; i++) {
            gravityTracker.addRecord(100);
        }

        assert.equal(gravityTracker.stable(12), true);
        assert.equal(gravityTracker.stable(24), true);
        assert.equal(gravityTracker.stable(48), true);
    });

    test("stable detects instability", () => {
        gravityTracker.init();

        for (let i = 0; i < 47; i++) {
            gravityTracker.addRecord(200);
        }
        gravityTracker.addRecord(100); // last value different

        assert.equal(gravityTracker.stable(12), false);
    });
});
