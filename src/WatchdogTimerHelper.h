/**
 * @file  .
 * @brief Wrapper for ESP8266 watchdog‑timer.
 *
 * The ESP32 does **not** use this hardware watchdog; it runs FreeRTOS with its
 * own task‑watchdog mechanism, so the ESP8266‑specific functions (`wdtEnable`,
 * `wdtDisable`) are unavailable.  To keep the same source code compiling on
 * both platforms we provide inline functions that become *no‑ops* on the
 * ESP32.
 *
 * Only the two ESP8266 SDK calls needed in the project are wrapped:
 *   - `wdtEnable(timeoutMs)` – start the watchdog with a custom timeout
 *   - `wdtDisable()`         – stop the watchdog (used to restore the default)
 *
 * Typical usage pattern:
 * @code
 *   bpl::watchdog::disable();          // disables the hardware watchdog (ESP8266 only)
 *   // …perform blocking I/O
 *   bpl::watchdog::enable();           // back to the normal watchdog behavior
 * @endcode
 */

#ifndef BPL_WATCHDOG_TIMER_HELPER_H
#define BPL_WATCHDOG_TIMER_HELPER_H

#include <cstdint>

namespace bpl::watchdog {
    inline void enable(std::uint32_t timeoutMs = 3000);
    inline void disable();

#if defined(ESP8266)

#include <Esp.h>

    inline void disable()
    {
        EspClass::wdtDisable();
    }

    inline void enable(const std::uint32_t timeoutMs)
    {
        EspClass::wdtEnable(timeoutMs);
    }

#else

    inline void enable(std::uint32_t) { /* no‑op */ }
    inline void disable() { /* no‑op */ }

#endif
}

#endif
