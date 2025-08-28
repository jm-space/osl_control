# pragma once

# include "defines.hpp"

/// Get the hertz from a given rpm
# define hz_from_rpm(__rpm) (float)(__rpm / 60.0)

# define rad_from_rpm(__rpm) (float)(__rpm / 60.0 * 2.0 * )