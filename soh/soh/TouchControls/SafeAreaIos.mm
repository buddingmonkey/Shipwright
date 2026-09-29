#include "SafeAreaIos.h"

#import <UIKit/UIKit.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <imgui.h>

namespace {

TouchControlsSafeArea sCached = {};

UIView* GameView() {
    const Uint32 id = (Uint32)(intptr_t)ImGui::GetMainViewport()->PlatformHandle;
    SDL_Window* window = id != 0 ? SDL_GetWindowFromID(id) : nullptr;
    if (window == nullptr) {
        window = SDL_GetKeyboardFocus();
    }
    if (window == nullptr) {
        return nil;
    }
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    if (!SDL_GetWindowWMInfo(window, &info) || info.subsystem != SDL_SYSWM_UIKIT || info.info.uikit.window == nil) {
        return nil;
    }
    UIWindow* uiWindow = info.info.uikit.window;
    UIView* view = uiWindow.rootViewController.view;
    return view != nil ? view : uiWindow;
}

UIEdgeInsets UnusableInsets(UIView* view) {
    return view.safeAreaInsets;
}

} // namespace

TouchControlsSafeArea TouchControls_IosSafeArea() {
    if (!NSThread.isMainThread) {
        return sCached;
    }
    UIView* view = GameView();
    const CGFloat height = view.bounds.size.height;
    if (view == nil || height <= 0.0) {
        return sCached;
    }
    const UIEdgeInsets in = UnusableInsets(view);
    sCached = { (float)(in.left / height), (float)(in.top / height), (float)(in.right / height),
                (float)(in.bottom / height) };
    return sCached;
}
