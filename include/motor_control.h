#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include "context.h"

// Frequency converter motor interface
bool controlMotorSpeed(double currentRpm, double targetRpm);
void getLauncherState(Context& ctx); // Reads hall sensor for RPM and angle

#endif // MOTOR_CONTROL_H