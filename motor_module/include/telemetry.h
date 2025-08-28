#ifndef TELEMETRY_H
#define TELEMETRY_H

#include "context.h"
#include "types.h"

// LoRa telemetry interface
void setupTelemetry();
bool checkTelemetryHealth();
TelemetryCmd rcvCommand(const Context& ctx);
void broadcastState(const Context& ctx);
void rcvLaunchParameters(LaunchParameters& params);

#endif // TELEMETRY_H