#pragma once

#ifdef ENABLE_DEBUG_TOOLS

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void DebugPad_MergeInto(void* contPad);
bool DebugPad_TakeMenuToggle(void);

#ifdef __cplusplus
}
#endif

#endif
