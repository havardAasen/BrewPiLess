class GravityTracker {
    readonly NumberOfSlots: number = 48;

    private record: number[] = [];
    private ridx = 0;
    private threshold = 1;
    private currentStart = 0;
    private lastValue = 0;

    setThreshold(threshold: number): void {
        this.threshold = threshold;
    }

    addRecord(v: number): void {
        this.record[this.ridx++] = v;
        if (this.ridx >= this.NumberOfSlots) {
            this.ridx = 0;
        }
    }

    stable(duration: number, to?: number): boolean {
        const threshold = typeof to === "undefined" ? this.threshold : to;

        let current = this.ridx - 1;
        if (current < 0) current = this.NumberOfSlots - 1;

        let previous = this.NumberOfSlots + this.ridx - duration;
        while (previous >= this.NumberOfSlots) {
            previous -= this.NumberOfSlots;
        }

        return this.record[previous] - this.record[current] <= threshold;
    }

    init(): void {
        this.currentStart = 0;
        this.lastValue = 0;
    }

    add(gravity: number, time: number): void {
        const Period = 3600;
        const timediff = time - this.currentStart;

        if (timediff > Period) {
            this.addRecord(gravity);

            if (this.lastValue !== 0) {
                let diff = timediff - Period;
                while (diff > Period) {
                    diff -= Period;
                    this.addRecord(this.lastValue);
                }
            }

            this.currentStart = time;
            this.lastValue = gravity;
        }
    }
}

export const gravityTracker = new GravityTracker();
