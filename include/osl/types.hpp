# pragma once

namespace osl {
    enum class State {
        DISARMED,
        ARMING,
        ARMED,
        SPINUP,
        READY,
        ENGAGE_TRIGGER,
        RELEASE_FIRST,
        RELEASE_SECOND,
        SPINDOWN,
        ABORT
    };

    /*
    enum class TelemetryCmd {
        NONE,
        ABORT,
        ARM,
        DISARM,
        SPINUP,
        RELEASE
    };
    */

    struct LaunchParam {
        float rpm;
        float angle;
    };
}