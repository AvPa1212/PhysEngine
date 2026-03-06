/**
 * EventBridge – Pub/sub system for non-blocking engine→UI event communication.
 *
 * The C++ engine "emits" events (e.g., EntropyThresholdReached, SystemOverheat)
 * and the React layer subscribes without the engine needing to know how the UI
 * is implemented.
 */
export class EventBridge {
  constructor() {
    /** @type {Map<string, Set<Function>>} */
    this._listeners = new Map();
  }

  /**
   * Subscribe to an event type.
   * @param {string} eventType - Event name (e.g. 'EntropyThresholdReached')
   * @param {Function} callback - Handler receiving the event payload
   * @returns {Function} Unsubscribe function
   */
  subscribe(eventType, callback) {
    if (!this._listeners.has(eventType)) {
      this._listeners.set(eventType, new Set());
    }
    this._listeners.get(eventType).add(callback);

    return () => {
      const set = this._listeners.get(eventType);
      if (set) {
        set.delete(callback);
        if (set.size === 0) this._listeners.delete(eventType);
      }
    };
  }

  /**
   * Emit an event to all subscribers.
   * @param {string} eventType
   * @param {*} data - Event payload
   */
  emit(eventType, data) {
    const callbacks = this._listeners.get(eventType);
    if (callbacks) {
      callbacks.forEach((cb) => {
        try {
          cb(data);
        } catch (err) {
          // Swallow subscriber errors to avoid breaking the event loop
          console.error(`[EventBridge] Error in ${eventType} handler:`, err);
        }
      });
    }
  }

  /** Remove all listeners. */
  clear() {
    this._listeners.clear();
  }
}
