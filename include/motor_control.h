#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include "context.h"

bool controlMotorSpeed(double currentRpm, double targetRpm);

void getLauncherState(Context& ctx);

#endif // MOTOR_CONTROL_H