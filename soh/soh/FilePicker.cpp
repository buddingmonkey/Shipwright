#include "FilePicker.h"

#if defined(__ANDROID__)
#include <jni.h>
#include <mutex>
#include <SDL2/SDL_system.h>
#include <spdlog/spdlog.h>
#elif defined(__IOS__)
#include <cstdlib>
#include <sys/sysctl.h>
#elif !defined(__SWITCH__) && !defined(__WIIU__)
#define SOH_PFD_PICKER
#include "Extractor/portable-file-dialogs.h"
#endif

namespace fs = std::filesystem;

#ifdef __ANDROID__
namespace {
std::mutex sPickMutex;
SohFilePicker::Callback sPickCallback;
std::optional<fs::path> sPickResult;
bool sPickPending = false;
bool sPickReady = false;
} // namespace

extern "C" JNIEXPORT void JNICALL Java_com_harbormasters_soh_SohActivity_nativeFilePicked(JNIEnv* env, jclass,
                                                                                          jstring path) {
    std::optional<fs::path> picked;
    if (path != nullptr) {
        const char* chars = env->GetStringUTFChars(path, nullptr);
        if (chars != nullptr) {
            picked = fs::path(chars);
            env->ReleaseStringUTFChars(path, chars);
        }
    }
    std::lock_guard<std::mutex> lock(sPickMutex);
    sPickResult = std::move(picked);
    sPickReady = true;
}

static bool OpenAndroidPicker() {
    JNIEnv* env = static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
    jobject activity = static_cast<jobject>(SDL_AndroidGetActivity());
    if (env == nullptr || activity == nullptr) {
        SPDLOG_ERROR("No Android activity to open the file picker on");
        return false;
    }
    jclass activityClass = env->GetObjectClass(activity);
    jmethodID open = env->GetMethodID(activityClass, "openFilePicker", "()V");
    const bool found = open != nullptr;
    if (found) {
        env->CallVoidMethod(activity, open);
    } else {
        env->ExceptionClear();
        SPDLOG_ERROR("SohActivity.openFilePicker is missing");
    }
    env->DeleteLocalRef(activityClass);
    env->DeleteLocalRef(activity);
    return found;
}
#endif

namespace SohFilePicker {

bool IsAvailable() {
#if defined(__ANDROID__) || defined(SOH_PFD_PICKER)
    return true;
#else
    return false;
#endif
}

void PickFile(const std::string& title, const std::vector<std::string>& filters, Callback onResult) {
#if defined(__ANDROID__)
    {
        std::lock_guard<std::mutex> lock(sPickMutex);
        if (sPickPending) {
            return;
        }
        sPickCallback = std::move(onResult);
        sPickResult.reset();
        sPickPending = true;
        sPickReady = false;
    }
    if (!OpenAndroidPicker()) {
        std::lock_guard<std::mutex> lock(sPickMutex);
        sPickReady = true;
    }
#elif defined(SOH_PFD_PICKER)
    std::vector<std::string> selection = pfd::open_file(title, "", filters).result();
    if (selection.empty()) {
        onResult(std::nullopt);
    } else {
        onResult(fs::path(selection[0]));
    }
#else
    onResult(std::nullopt);
#endif
}

void Pump() {
#ifdef __ANDROID__
    Callback callback;
    std::optional<fs::path> result;
    {
        std::lock_guard<std::mutex> lock(sPickMutex);
        if (!sPickReady) {
            return;
        }
        callback = std::move(sPickCallback);
        sPickCallback = nullptr;
        result = std::move(sPickResult);
        sPickResult.reset();
        sPickPending = false;
        sPickReady = false;
    }
    if (callback) {
        callback(result);
    }
#endif
}

bool IsStagedCopy(const fs::path& path) {
#ifdef __ANDROID__
    const char* internal = SDL_AndroidGetInternalStoragePath();
    if (internal == nullptr) {
        return false;
    }
    std::error_code ec;
    return fs::equivalent(path.parent_path(), fs::path(internal) / "import", ec);
#else
    return false;
#endif
}

std::string FilesAppFolder() {
#if defined(SOH_VISIONOS)
    return "Files > On My Apple Vision Pro > Ship of Harkinian";
#elif defined(__IOS__)
    const char* model = std::getenv("SIMULATOR_MODEL_IDENTIFIER");
    char machine[64] = {};
    size_t size = sizeof(machine) - 1;
    if (model == nullptr && sysctlbyname("hw.machine", machine, &size, nullptr, 0) == 0) {
        model = machine;
    }
    const bool iPhone = model != nullptr && std::string(model).rfind("iPhone", 0) == 0;
    return std::string("Files > ") + (iPhone ? "On My iPhone" : "On My iPad") + " > Ship of Harkinian";
#else
    return "";
#endif
}

} // namespace SohFilePicker
