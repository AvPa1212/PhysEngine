/**
 * PerformanceMonitor – Silent telemetry for tracking engine performance.
 *
 * Tracks:
 *  - Frame time (inter-frame interval)
 *  - C++ execution time per tick
 *  - Memory usage (when available)
 *
 * Uses a rolling window to keep memory bounded.
 */
export class PerformanceMonitor {
  private _windowSize: number;
  private _frameTimes: number[];
  private _execTimes: number[];
  private _memoryUsage: number[];
  private _lastTimestamp: number;

  /**
   * @param {number} windowSize - Number of samples to retain (default: 60)
   */
  constructor(windowSize = 60) {
    this._windowSize = windowSize;
    this._frameTimes = [];
    this._execTimes = [];
    this._memoryUsage = [];
    this._lastTimestamp = 0;
  }

  /**
   * Record a performance sample from the physics worker.
   * @param {{ execTime: number, taskCount: number }} sample
   */
  record(sample: { execTime?: number; taskCount?: number }) {
    const now = performance.now();

    // Track inter-frame time
    if (this._lastTimestamp > 0) {
      this._frameTimes.push(now - this._lastTimestamp);
      if (this._frameTimes.length > this._windowSize) this._frameTimes.shift();
    }
    this._lastTimestamp = now;

    // Track C++ execution time
    if (typeof sample.execTime === 'number') {
      this._execTimes.push(sample.execTime);
      if (this._execTimes.length > this._windowSize) this._execTimes.shift();
    }

    // Track memory usage (Chrome-only API)
    const perfWithMemory = performance as Performance & {
      memory?: { usedJSHeapSize: number };
    };
    if (typeof performance !== 'undefined' && perfWithMemory.memory) {
      this._memoryUsage.push(perfWithMemory.memory.usedJSHeapSize);
      if (this._memoryUsage.length > this._windowSize) this._memoryUsage.shift();
    }
  }

  /**
   * Compute summary metrics from the rolling window.
   * @returns {{ fps: number, avgFrameTimeMs: number, avgExecTimeMs: number, avgMemoryMB: number }}
   */
  getMetrics() {
    const avg = (arr: number[]) =>
      arr.length > 0 ? arr.reduce((a, b) => a + b, 0) / arr.length : 0;

    const avgFrameTime = avg(this._frameTimes);
    const fps = avgFrameTime > 0 ? 1000 / avgFrameTime : 0;

    return {
      fps: Math.round(fps * 100) / 100,
      avgFrameTimeMs: Math.round(avgFrameTime * 1000) / 1000,
      avgExecTimeMs: Math.round(avg(this._execTimes) * 1000) / 1000,
      avgMemoryMB:
        Math.round((avg(this._memoryUsage) / (1024 * 1024)) * 100) / 100,
    };
  }

  /** Reset all counters. */
  reset() {
    this._frameTimes = [];
    this._execTimes = [];
    this._memoryUsage = [];
    this._lastTimestamp = 0;
  }
}
