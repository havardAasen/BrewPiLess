class GravityFilter {
    private b = 0.1;
    private y = 0;

    reset(): void {
        this.y = 0;
    }

    add(gravity: number): number {
        if (this.y === 0) {
            this.y = gravity;
        } else {
            this.y = this.y + this.b * (gravity - this.y);
        }
        return Math.round(this.y * 10000) / 10000;
    }

    setBeta(beta: number): void {
        this.b = beta;
    }
}

export const gravityFilter = new GravityFilter();
