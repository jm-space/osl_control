#include "telemetry.h"

void setupTelemetry(){
    // TODO: Establish telemetry link
}

bool checkTelemetryHealth(){
    // TODO: Check connection state
    return false;
}

TelemetryCmd rcvCommand(const Context& /*ctx*/) {
    // TODO: Read command from radio link

    return TelemetryCmd::NONE;
}

void broadcastState(const Context& /*ctx*/) {
    // TODO: Send ctx.launcherState over your telemetry
}

void rcvLaunchParameters(LaunchParameters& /*params*/) {
    // TODO: Read rpmRelease and angleRelease from telemetry and store in params
}