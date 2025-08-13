#include "state_machine.h"
#include "config.h"
#include "telemetry.h"
#include "motor_control.h"
#include "trigger.h"
#include <Arduino.h> // for millis()

void fsm_loop(Context& ctx, uint32_t dt) {
    static uint32_t timeSinceLastBroadcast = 0;
    static uint32_t timeEnteredReady     = 0;
    static uint32_t triggerStartTime     = 0;
    static uint32_t timeNoTelemetry = 0;

    getLauncherState(ctx);
    TelemetryCmd telCmd = rcvCommand(ctx);

    timeSinceLastBroadcast += dt;
    if (timeSinceLastBroadcast >= BROADCAST_INTERVAL_MS) {
        broadcastState(ctx);
        timeSinceLastBroadcast = 0;
    }

    switch (ctx.launcherState.state) {
        case State::NO_TELEMETRY:
            if(checkTelemetryHealth()){
                timeNoTelemetry = 0;
                ctx.launcherState.state = State::DISARMED;
            }

            timeNoTelemetry += dt;
            if(timeNoTelemetry >= TELEMETRY_RETRY_INTERVAL_MS){
                setupTelemetry();
            }
            break;
        case State::DISARMED:
            if (telCmd == TelemetryCmd::ARM) {
                armTrigger(ctx.launchParams);
                ctx.launcherState.state = State::ARMING;
            }
            rcvLaunchParameters(ctx.launchParams);
            break;

        case State::ARMING:
            if (telCmd == TelemetryCmd::ABORT || telCmd == TelemetryCmd::DISARM) {
                ctx.launcherState.state = State::DISARMED;
            }
            else if (triggerReady(ctx)) {
                ctx.launcherState.state = State::ARMED;
            }
            break;

        case State::ARMED:
            if (telCmd == TelemetryCmd::ABORT || telCmd == TelemetryCmd::DISARM) {
                ctx.launcherState.state = State::DISARMED;
            }
            else if (telCmd == TelemetryCmd::SPINUP) {
                ctx.launcherState.state = State::SPINUP;
            }
            break;

        case State::SPINUP:
            if (telCmd == TelemetryCmd::ABORT) {
                ctx.launcherState.state = State::ABORT;
            }
            else if (controlMotorSpeed(
                        ctx.launcherState.rpm,
                        ctx.launchParams.rpmRelease
                     )) {
                timeEnteredReady = millis();
                ctx.launcherState.state = State::READY;
            }
            break;

        case State::READY: {
            uint32_t now = millis();
            if (telCmd == TelemetryCmd::ABORT ||
                (now - timeEnteredReady) > MAX_READY_WAIT_MS) {
                ctx.launcherState.state = State::ABORT;
            }
            else if (
                controlMotorSpeed(ctx.launcherState.rpm,
                                  ctx.launchParams.rpmRelease)
                && telCmd == TelemetryCmd::RELEASE) {
                ctx.launcherState.state = State::RELEASE_FIRST;
            }
            break;
        }

        case State::ENGAGE_TRIGGER:
            if (telCmd == TelemetryCmd::ABORT) {
                ctx.launcherState.state = State::ABORT;
            }
            else if (engageTrigger(ctx)) {
                triggerStartTime = millis();
                ctx.launcherState.state = State::RELEASE_FIRST;
            }
            break;

        case State::RELEASE_FIRST: {
            uint32_t now = millis();
            double halfRotMs = (30.0 / ctx.launcherState.rpm) * 1000.0; // Half rotation
            if ((now - triggerStartTime) >= static_cast<uint32_t>(halfRotMs)) {
                ctx.launcherState.state = State::RELEASE_SECOND;
            }

            controlMotorSpeed(ctx.launcherState.rpm, ctx.launchParams.rpmRelease);
            break;
        }

        case State::RELEASE_SECOND: {
            uint32_t now = millis();
            double fullRotMs = (60.0 / ctx.launcherState.rpm) * 1000.0; // Full rotation
            if ((now - triggerStartTime) >= static_cast<uint32_t>(fullRotMs)) {
                ctx.launcherState.state = State::SPINDOWN;
            }
            controlMotorSpeed(ctx.launcherState.rpm, ctx.launchParams.rpmRelease);
            break;
        }

        case State::SPINDOWN:
            if (controlMotorSpeed(ctx.launcherState.rpm, 0.0)) {
                ctx.launcherState.state = State::DISARMED;
            }
            break;

        case State::ABORT:
            // Redirect to spindown
            ctx.launcherState.state = State::SPINDOWN;
            break;
    }
}
