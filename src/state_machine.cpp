#include <Arduino.h> // for millis()

#include "state_machine.h"
#include "config.h"
#include "telemetry.h"
#include "motor_control.h"
#include "trigger.h"

// Helper for timing
static uint32_t millis_since(uint32_t start) {
    return millis() - start;
}

void fsm_loop(Context& ctx, uint32_t dt) {
    static uint32_t lastBroadcast = 0;
    static uint32_t readyEnteredAt = 0;
    static uint32_t triggerEngagedAt = 0;
    static uint32_t telemetryLostAt = 0;
    static bool telemetryWasHealthy = true;
    static bool telemetryLossReacted = false;

    // Update launcher state from sensors
    getLauncherState(ctx);

    // Check telemetry health
    bool telemetryHealthy = checkTelemetryHealth();
    TelemetryCmd telCmd = telemetryHealthy ? rcvCommand(ctx) : TelemetryCmd::NONE;

        // Handle telemetry loss
    if (!telemetryHealthy) {
        if (telemetryWasHealthy) {
            telemetryLostAt = millis();
            telemetryWasHealthy = false;
        }
        // Wait for max telemetry timeout before taking action
        if (millis_since(telemetryLostAt) > MAX_NO_TELEMETRY_WAIT_MS) {
            // Reconnect
            setupTelemetry();
            if(!telemetryLossReacted){
                switch (ctx.launcherState.state) {
                    case State::SPINUP:
                    case State::READY:
                    case State::ENGAGE_TRIGGER:
                    case State::RELEASE_FIRST:
                    case State::RELEASE_SECOND:
                        // Transition to SPINDOWN state, if connection is lost during "active" phase
                        ctx.launcherState.state = State::SPINDOWN;
                        break;
                    default:
                        ctx.launcherState.state = State::DISARMED;
                        break;
                    }
                telemetryLossReacted = true;
            }
        }
    } else {
        telemetryLossReacted = false;
        telemetryWasHealthy = true;
    }

    // Broadcast state
    lastBroadcast += dt;
    if (lastBroadcast >= BROADCAST_INTERVAL_MS) {
        broadcastState(ctx);
        lastBroadcast = 0;
    }

    switch (ctx.launcherState.state) {
        case State::DISARMED:
            rcvLaunchParameters(ctx.launchParams);
            if (telCmd == TelemetryCmd::ARM) {
                armTrigger(ctx.launchParams);
                ctx.launcherState.state = State::ARMING;
            }
            break;

        case State::ARMING:
            if (telCmd == TelemetryCmd::DISARM || telCmd == TelemetryCmd::ABORT) {
                retractTrigger(ctx);
                ctx.launcherState.state = State::DISARMED;
            } else if (triggerReady(ctx)) {
                ctx.launcherState.state = State::ARMED;
            }
            break;

        case State::ARMED:
            if (telCmd == TelemetryCmd::DISARM || telCmd == TelemetryCmd::ABORT) {
                retractTrigger(ctx);
                ctx.launcherState.state = State::DISARMED;
            } else if (telCmd == TelemetryCmd::SPINUP) {
                ctx.launcherState.state = State::SPINUP;
            }
            break;

        case State::SPINUP:
            if (telCmd == TelemetryCmd::ABORT) {
                ctx.launcherState.state = State::ABORT;
            } else if (controlMotorSpeed(ctx.launcherState.rpm, ctx.launchParams.rpmRelease)) {
                readyEnteredAt = millis();
                ctx.launcherState.state = State::READY;
            }
            break;

        case State::READY: {
            // Wait for release or timeout
            uint32_t elapsed = millis_since(readyEnteredAt);
            if (telCmd == TelemetryCmd::ABORT || elapsed > MAX_READY_WAIT_MS) {
                ctx.launcherState.state = State::ABORT;
            } else if (telCmd == TelemetryCmd::RELEASE &&
                       controlMotorSpeed(ctx.launcherState.rpm, ctx.launchParams.rpmRelease)) {
                ctx.launcherState.state = State::ENGAGE_TRIGGER;
            }
            break;
        }

        case State::ENGAGE_TRIGGER:
            if (telCmd == TelemetryCmd::ABORT) {
                ctx.launcherState.state = State::ABORT;
            } else if (engageTrigger(ctx)) {
                triggerEngagedAt = millis();
                ctx.launcherState.state = State::RELEASE_FIRST;
            }
            break;

        case State::RELEASE_FIRST: {
            // Release first rocket after ramp deployed and arm passes ramp
            double halfRotMs = (30.0 / ctx.launcherState.rpm) * 1000.0;
            if (millis_since(triggerEngagedAt) >= static_cast<uint32_t>(halfRotMs)) {
                ctx.launcherState.state = State::RELEASE_SECOND;
            }
            controlMotorSpeed(ctx.launcherState.rpm, ctx.launchParams.rpmRelease);
            break;
        }

        case State::RELEASE_SECOND: {
            // Release second rocket after another half rotation
            double fullRotMs = (60.0 / ctx.launcherState.rpm) * 1000.0;
            if (millis_since(triggerEngagedAt) >= static_cast<uint32_t>(fullRotMs)) {
                ctx.launcherState.state = State::SPINDOWN;
            }
            controlMotorSpeed(ctx.launcherState.rpm, ctx.launchParams.rpmRelease);
            break;
        }

        case State::SPINDOWN:
            if (controlMotorSpeed(ctx.launcherState.rpm, 0.0)) {
                retractTrigger(ctx);
                ctx.launcherState.state = State::DISARMED;
            }
            break;

        case State::ABORT:
            // Immediately spin down and retract trigger
            retractTrigger(ctx);
            controlMotorSpeed(ctx.launcherState.rpm, 0.0);
            ctx.launcherState.state = State::SPINDOWN;
            break;
    }
}
