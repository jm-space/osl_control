#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>

constexpr uint32_t BROADCAST_INTERVAL_MS = 200;

constexpr uint32_t MAX_READY_WAIT_MS = 10 * 1000;

constexpr uint32_t TELEMETRY_RETRY_INTERVAL_MS = 2 * 1000;

constexpr uint32_t MAX_NO_TELEMETRY_WAIT_MS = 1 * 1000;

#endif // CONFIG_H