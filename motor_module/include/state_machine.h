#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <cstdint>
#include "context.h"

void fsm_loop(Context& ctx, uint32_t dt);

#endif // STATE_MACHINE_H