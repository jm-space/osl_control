#include <Arduino.h>
#include "config.h"
#include "context.h"
#include "state_machine.h"

Context ctx;  // Globally shared context

void setup() {
    Serial.begin(115200);

    ctx.launcherState.state = State::DISARMED;
    ctx.launcherState.rpm   = 0.0;
    ctx.launcherState.angle = 0.0;
}

void loop() {
    static uint32_t lastTime = millis();
    uint32_t now = millis();
    uint32_t dt  = now - lastTime;
    lastTime = now;

    fsm_loop(ctx, dt);
}