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
#include <ship/Context.h>

#include "soh/cvar_prefixes.h"

namespace {

constexpr int RATE_SETTLE_TICKS = 90;

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

#ifdef ENABLE_XR_WINDOW

extern "C" void XrWindow_BeginFlat(Gfx** gfx) {
    gSPXrFlatProjection((*gfx)++, 1);
}

extern "C" void XrWindow_EndFlat(Gfx** gfx) {
    gSPXrFlatProjection((*gfx)++, 0);
}

extern "C" void XrWindow_BeginSceneDepth(Gfx** gfx) {
    gSPXrSceneDepth((*gfx)++, 1);
}

extern "C" void XrWindow_EndSceneDepth(Gfx** gfx) {
    gSPXrSceneDepth((*gfx)++, 0);
}

#else

extern "C" void XrWindow_BeginFlat(Gfx** gfx) {
}

extern "C" void XrWindow_EndFlat(Gfx** gfx) {
}

extern "C" void XrWindow_BeginSceneDepth(Gfx** gfx) {
}

extern "C" void XrWindow_EndSceneDepth(Gfx** gfx) {
}

#endif
