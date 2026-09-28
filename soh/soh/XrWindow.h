#pragma once

#include <libultraship/libultra/gbi.h>

#ifdef __cplusplus
namespace Fast {
class Fast3dWindow;
}

namespace SoH {
bool IsHeadsetWindow();
void XrWindow_Sync(Fast::Fast3dWindow* wnd);
} // namespace SoH

extern "C" {
#endif

void XrWindow_BeginFlat(Gfx** gfx);
void XrWindow_EndFlat(Gfx** gfx);
void XrWindow_BeginSceneDepth(Gfx** gfx);
void XrWindow_EndSceneDepth(Gfx** gfx);

#ifdef __cplusplus
}
#endif
