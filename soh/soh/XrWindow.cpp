#include "XrWindow.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <vector>

#include <fast/Fast3dWindow.h>
#ifdef ENABLE_XR_WINDOW
#include <fast/backends/gfx_xr_view.h>
#endif
#include <libultraship/bridge/consolevariablebridge.h>
#include <libultraship/libultra/controller.h>
#include <ship/Context.h>
#include <ship/controller/controldeck/ControlDeck.h>

#include "soh/cvar_prefixes.h"

namespace {

constexpr int RATE_SETTLE_TICKS = 90;
constexpr float STICK_RANGE = 80.0f;
constexpr float PULL_POINT = 0.66f;
constexpr float C_POINT = 0.5f;
constexpr float MENU_ANGLE_PER_UNIT = 0.0009f;
constexpr float MENU_ANGLE_MIN = 0.75f;
constexpr float SKY_CORNER = 218.3f;
constexpr float SKY_REACH = 0.9f;

void SelectRefreshRate(Fast::Fast3dWindow* wnd) {
    const int cap = CVarGetInteger(CVAR_SETTING("XrMaxRate"), 120);

    std::vector<float> rates;
    for (float rate : wnd->GetSupportedRefreshRates()) {
        if (rate <= (float)cap) {
            rates.push_back(rate);
        }
    }
    if (rates.empty()) {
        return;
    }
    std::sort(rates.begin(), rates.end(), std::greater<float>());

    static int askedCap = -1;
    static float asked = 0.0f;
    static int waited = 0;
    if (askedCap != cap) {
        askedCap = cap;
        asked = 0.0f;
        waited = 0;
    }

    if (asked <= 0.0f) {
        asked = rates.front();
        wnd->SetRefreshRate(asked);
        waited = 0;
        return;
    }

    if (fabsf((float)wnd->GetCurrentRefreshRate() - asked) < 0.5f) {
        waited = 0;
        return;
    }
    if (++waited < RATE_SETTLE_TICKS) {
        return;
    }
    waited = 0;
    for (size_t i = 0; i + 1 < rates.size(); i++) {
        if (fabsf(rates[i] - asked) < 0.5f) {
            asked = rates[i + 1];
            wnd->SetRefreshRate(asked);
            return;
        }
    }
}

#ifdef ENABLE_OPENXR
bool SyncSetting(const char* cVar, float low, float high, float defaultValue, float& pushed, float held,
                 void (*apply)(float)) {
    const float shown = std::clamp(CVarGetFloat(cVar, defaultValue), low, high);
    if (shown != pushed) {
        apply(shown);
        pushed = shown;
        return false;
    }
    const float left = std::clamp(held, low, high);
    if (fabsf(left - shown) > 0.001f) {
        CVarSetFloat(cVar, left);
        pushed = left;
        return true;
    }
    return false;
}
#endif

} // namespace

namespace SoH {

bool IsHeadsetWindow() {
#ifdef ENABLE_XR_WINDOW
    return Fast::IsXrPresenting();
#else
    return false;
#endif
}

bool XrWindow_MenuScale(float* scale) {
#ifdef ENABLE_XR_WINDOW
    if (!IsHeadsetWindow()) {
        return false;
    }
    auto window = Ship::Context::GetRawInstance()->GetWindow();
    const float angularWidth = Fast::GetXrWindowAngularWidth();
    if (angularWidth <= 0.0f || window == nullptr || window->GetWidth() == 0) {
        return false;
    }
    *scale = MENU_ANGLE_PER_UNIT * (float)window->GetWidth() / std::max(angularWidth, MENU_ANGLE_MIN);
    return true;
#else
    return false;
#endif
}

void XrWindow_Sync(Fast::Fast3dWindow* wnd) {
    if (wnd == nullptr || !IsHeadsetWindow()) {
        return;
    }

    SelectRefreshRate(wnd);

#ifdef ENABLE_XR_WINDOW
    Fast::SetXrDioramaDepth(CVarGetFloat(CVAR_SETTING("XrDioramaDepth"), 2.0f));
#endif

#ifdef ENABLE_OPENXR
    static float pushedRange = 0.0f;
    static float pushedScale = 0.0f;
    const bool rangeMoved = SyncSetting(CVAR_SETTING("XrWindowRange"), 0.5f, 4.0f, 1.3f, pushedRange,
                                        Fast::GetXrWindowDistance(), Fast::SetXrWindowDistance);
    const bool scaleMoved = SyncSetting(CVAR_SETTING("XrWindowScale"), 0.5f, 8.0f, 2.6f, pushedScale,
                                        Fast::GetXrWindowScale(), Fast::SetXrWindowScale);

    static bool wasMoving = false;
    const bool moving = rangeMoved || scaleMoved;
    if (wasMoving && !moving) {
        Ship::Context::GetRawInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
    }
    wasMoving = moving;

    wnd->SetResolutionMultiplier(CVarGetFloat(CVAR_INTERNAL_RESOLUTION, 1.0f));
    Fast::SetXrStereo(CVarGetInteger(CVAR_SETTING("XrStereo"), 1) != 0);
    Fast::SetXrEdgeSoftness(CVarGetFloat(CVAR_SETTING("XrEdgeSoftness"), 0.36f));
    Fast::SetXrEdgeFloat(CVarGetFloat(CVAR_SETTING("XrEdgeFloat"), 0.15f));
#endif
}

} // namespace SoH

extern "C" bool XrWindow_IsHeadset(void) {
    return SoH::IsHeadsetWindow();
}

extern "C" float XrWindow_SkyScale(float zFar) {
    if (!SoH::IsHeadsetWindow() || zFar <= SKY_CORNER) {
        return 0.0f;
    }
    return SKY_REACH * zFar / SKY_CORNER;
}

#ifdef ENABLE_XR_WINDOW

extern "C" void XrWindow_BeginFlat(Gfx** gfx) {
    gSPXrFlatProjection((*gfx)++, 1);
}

extern "C" void XrWindow_EndFlat(Gfx** gfx) {
    gSPXrFlatProjection((*gfx)++, 0);
}

extern "C" void XrWindow_BeginUnmeasured(Gfx** gfx) {
    gSPXrSceneDepth((*gfx)++, 1);
}

extern "C" void XrWindow_EndUnmeasured(Gfx** gfx) {
    gSPXrSceneDepth((*gfx)++, 0);
}

#else

extern "C" void XrWindow_BeginFlat(Gfx** gfx) {
}

extern "C" void XrWindow_EndFlat(Gfx** gfx) {
}

extern "C" void XrWindow_BeginUnmeasured(Gfx** gfx) {
}

extern "C" void XrWindow_EndUnmeasured(Gfx** gfx) {
}

#endif

#ifdef ENABLE_OPENXR

extern "C" void XrWindow_MergePad(void* contPad) {
    Fast::XrPadState xr;
    if (contPad == nullptr || !Fast::GetXrPad(&xr)) {
        return;
    }
    auto ctx = Ship::Context::GetRawInstance();
    if (ctx == nullptr || ctx->GetControlDeck() == nullptr || ctx->GetControlDeck()->GamepadGameInputBlocked()) {
        return;
    }

    uint32_t buttons = 0;
    if (xr.buttons & Fast::XR_PAD_A) {
        buttons |= BTN_A;
    }
    if (xr.buttons & Fast::XR_PAD_B) {
        buttons |= BTN_B;
    }
    if (xr.buttons & Fast::XR_PAD_MENU) {
        buttons |= BTN_START;
    }
    if (xr.buttons & Fast::XR_PAD_X) {
        buttons |= BTN_DLEFT;
    }
    if (xr.buttons & Fast::XR_PAD_Y) {
        buttons |= BTN_DUP;
    }
    if (xr.trigger[0] >= PULL_POINT) {
        buttons |= BTN_Z;
    }
    if (xr.trigger[1] >= PULL_POINT || xr.squeeze[1] >= PULL_POINT) {
        buttons |= BTN_R;
    }
    if (xr.squeeze[0] >= PULL_POINT) {
        buttons |= BTN_L;
    }
    if (xr.stick[1][0] <= -C_POINT) {
        buttons |= BTN_CLEFT;
    }
    if (xr.stick[1][0] >= C_POINT) {
        buttons |= BTN_CRIGHT;
    }
    if (xr.stick[1][1] >= C_POINT) {
        buttons |= BTN_CUP;
    }
    if (xr.stick[1][1] <= -C_POINT) {
        buttons |= BTN_CDOWN;
    }

    OSContPad* pad = static_cast<OSContPad*>(contPad);
    pad->button |= buttons;
    if (pad->stick_x == 0 && pad->stick_y == 0) {
        pad->stick_x = (int8_t)std::lround(std::clamp(xr.stick[0][0], -1.0f, 1.0f) * STICK_RANGE);
        pad->stick_y = (int8_t)std::lround(std::clamp(xr.stick[0][1], -1.0f, 1.0f) * STICK_RANGE);
    }
    if (pad->right_stick_x == 0 && pad->right_stick_y == 0) {
        pad->right_stick_x = (int8_t)std::lround(std::clamp(xr.stick[1][0], -1.0f, 1.0f) * STICK_RANGE);
        pad->right_stick_y = (int8_t)std::lround(std::clamp(xr.stick[1][1], -1.0f, 1.0f) * STICK_RANGE);
    }
}

#else

extern "C" void XrWindow_MergePad(void* contPad) {
}

#endif
