#ifndef TRIGGER_H
#define TRIGGER_H

#include "context.h"

void armTrigger(const LaunchParameters& params);

bool triggerReady(const Context& ctx);

bool engageTrigger(Context& ctx);

void retractTrigger(Context& ctx);

#endif // TRIGGER_H