# pragma once 

/// Pin connected to the FWD-Pin of the AC-Driver
# define PIN_FWD_CONTROL 0
/// Pin connected to the BWD-Pin of the AC-Driver
# define PIN_BWD_CONTROL 0
/// Pin 
# define PIN_FREQ_PWM 0 

namespace motor_module {
    /// Current RPM of the spin launcher, will be backed up by a sensor soon
    extern float rpm;
    /// Target RPM of the spin launcher
    extern float target_rpm;

    /// Runs setup for all used pins etc.
    extern void setup();

    /// 
    /// @param dt 
    extern void loop(float dt);
}