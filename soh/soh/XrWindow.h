#pragma once

#include <libultraship/libultra/gbi.h>

#ifdef __cplusplus
namespace Fast {
class Fast3dWindow;
}

namespace SoH {
bool IsHeadsetWindow();
void XrWindow_Sync(Fast::Fast3dWindow* wnd);
bool XrWindow_MenuScale(float* scale);
} // namespace SoH

extern "C" {
#endif

void XrWindow_BeginFlat(Gfx** gfx);
void XrWindow_EndFlat(Gfx** gfx);
void XrWindow_BeginUnmeasured(Gfx** gfx);
void XrWindow_EndUnmeasured(Gfx** gfx);
void XrWindow_MergePad(void* contPad);
float XrWindow_SkyScale(float zFar);

#ifdef __cplusplus
}
#endif
