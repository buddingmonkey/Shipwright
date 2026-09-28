#include "ScenePrefetch.h"

#include <array>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <future>
#include <list>
#include <string>
#include <thread>
#include <vector>

#include <fast/resource/ResourceType.h>
#include <ship/Context.h>
#include <ship/resource/ResourceManager.h>
#include <spdlog/spdlog.h>

#include "ResourceManagerHelpers.h"

extern "C" {
#include "functions.h"
#include "variables.h"
}

namespace {
using ResourceFuture = std::shared_future<std::shared_ptr<Ship::IResource>>;

constexpr auto kHoldLimit = std::chrono::seconds(5);
constexpr int kSkyPaths = 24;
constexpr std::array<const char*, 8> kResidentDirs = {
    "alt/objects/gameplay_keep/",     "alt/objects/object_link_boy/",   "alt/objects/object_link_child/",
    "alt/textures/icon_item_static/", "alt/textures/do_action_static/", "alt/textures/parameter_static/",
    "alt/textures/vr_fine",           "alt/textures/vr_cloud",
};

struct Request {
    std::list<std::string> masks;
    std::vector<std::string> exact;
    std::string sceneDir;
    std::string roomPrefix;
    s32 room = -1;
    bool sceneLevel = true;
};

std::shared_future<std::vector<ResourceFuture>> sPlan;
std::vector<ResourceFuture> sLoads;
std::chrono::steady_clock::time_point sStart;
bool sActive = false;
bool sStarted = false;
Request sScene;
std::vector<std::string> sSky;

bool IsReady(const auto& future) {
    return future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

bool IsResident(const std::string& path) {
    for (const char* dir : kResidentDirs) {
        if (path.starts_with(dir)) {
            return true;
        }
    }
    return false;
}

bool Wanted(const Request& request, const std::string& path) {
    if (request.sceneDir.empty() || !path.starts_with(request.sceneDir)) {
        return true;
    }
    const std::string name = path.substr(request.sceneDir.size());
    if (!name.starts_with(request.roomPrefix)) {
        return request.sceneLevel;
    }
    const char* digits = name.c_str() + request.roomPrefix.size();
    char* end = nullptr;
    const long room = std::strtol(digits, &end, 10);
    return end != digits && room == request.room;
}

std::vector<ResourceFuture> Plan(const Request& request) {
    auto resourceManager = Ship::Context::GetRawInstance()->GetResourceManager();
    std::vector<ResourceFuture> loads;
    if (!request.masks.empty()) {
        auto files = resourceManager->GetArchiveManager()->ListFiles(request.masks, {});
        for (const auto& file : *files) {
            if (Wanted(request, file)) {
                loads.push_back(resourceManager->LoadResourceAsync(file, true));
            }
        }
    }
    for (const auto& file : request.exact) {
        loads.push_back(resourceManager->LoadResourceAsync(file, true));
    }
    return loads;
}

void AddPath(std::vector<std::string>& paths, const void* texture) {
    const char* path = static_cast<const char*>(texture);
    if (path != nullptr && std::strncmp(path, "__OTR__", 7) == 0) {
        paths.push_back(std::string("alt/") + (path + 7));
    }
}

std::vector<std::string> SkyPaths(PlayState* play, int ahead) {
    std::array<const char*, kSkyPaths> textures{};
    std::vector<std::string> paths;
    const int count = Environment_GetSkyboxPrefetch(play, ahead, textures.data(), kSkyPaths);
    for (int i = 0; i < count; i++) {
        AddPath(paths, textures[i]);
    }
    return paths;
}

void Background(Request request) {
    std::thread([request = std::move(request)]() {
        for (auto& load : Plan(request)) {
            load.wait();
        }
    }).detach();
}

Request BuildScene(PlayState* play) {
    Request request;
    request.room = play->roomCtx.curRoom.num;
    SceneTableEntry* scene = play->loadedScene;
    if (scene != nullptr && scene->sceneFile.fileName != nullptr) {
        const std::string fileName = scene->sceneFile.fileName;
        s32 sceneId = play->sceneNum;
        bool nonShared = (sceneId >= SCENE_DEKU_TREE && sceneId <= SCENE_ICE_CAVERN) ||
                         sceneId == SCENE_GERUDO_TRAINING_GROUND || sceneId == SCENE_INSIDE_GANONS_CASTLE;
        std::string version = nonShared ? (ResourceMgr_IsGameMasterQuest() ? "mq" : "nonmq") : "shared";
        request.sceneDir = "alt/scenes/" + version + "/" + fileName + "/";
        request.roomPrefix = fileName.substr(0, fileName.rfind("_scene")) + "_room_";
        request.masks.push_back(request.sceneDir + "*");
        const char* title = scene->titleFile.fileName;
        if (title != nullptr && title[0] != '\0' && std::strcmp(title, "none") != 0) {
            request.masks.push_back(std::string("alt/textures/") + title + "/*");
        }
    }
    for (s32 i = 0; i < play->objectCtx.num; i++) {
        s16 id = std::abs(play->objectCtx.status[i].id);
        if (id <= 0 || id >= OBJECT_ID_MAX) {
            continue;
        }
        const char* name = gObjectTable[id].fileName;
        if (name == nullptr || name[0] == '\0') {
            continue;
        }
        std::string dir = std::string("alt/objects/") + name + "/";
        if (!IsResident(dir)) {
            request.masks.push_back(dir + "*");
        }
    }
    for (auto& segment : play->skyboxCtx.textures) {
        for (void* texture : segment) {
            AddPath(request.exact, texture);
        }
    }
    for (void* palette : play->skyboxCtx.palettes) {
        AddPath(request.exact, palette);
    }
    for (auto& path : SkyPaths(play, 0)) {
        request.exact.push_back(std::move(path));
    }
    return request;
}
} // namespace

extern "C" void ScenePrefetch_Start(PlayState* play) {
    sActive = false;
    sStarted = false;
    auto resourceManager = Ship::Context::GetRawInstance()->GetResourceManager();
    if (!resourceManager->IsAltAssetsEnabled()) {
        return;
    }

    sScene = BuildScene(play);
    sStart = std::chrono::steady_clock::now();
    sLoads.clear();
    sPlan = std::async(std::launch::async, [request = sScene]() { return Plan(request); }).share();
    sActive = true;
    sStarted = true;
    sSky.clear();
}

extern "C" void ScenePrefetch_Room(int roomNum) {
    if (!sStarted || sScene.sceneDir.empty()) {
        return;
    }
    Request request;
    request.sceneDir = sScene.sceneDir;
    request.roomPrefix = sScene.roomPrefix;
    request.room = roomNum;
    request.sceneLevel = false;
    request.masks.push_back(request.sceneDir + "*");
    SPDLOG_INFO("scene prefetch: room {}", roomNum);
    Background(std::move(request));
}

extern "C" void ScenePrefetch_Sky(PlayState* play) {
    if (!sStarted) {
        return;
    }
    std::vector<std::string> paths = SkyPaths(play, 0);
    for (auto& path : SkyPaths(play, 1)) {
        paths.push_back(std::move(path));
    }
    if (paths == sSky) {
        return;
    }
    sSky = paths;
    Request request;
    request.exact = std::move(paths);
    Background(std::move(request));
}

extern "C" bool ScenePrefetch_Hold(void) {
    if (!sActive) {
        return false;
    }

    const auto elapsed = std::chrono::steady_clock::now() - sStart;
    bool done = false;
    if (IsReady(sPlan)) {
        if (sLoads.empty()) {
            sLoads = sPlan.get();
        }
        done = true;
        for (const auto& load : sLoads) {
            if (!IsReady(load)) {
                done = false;
                break;
            }
        }
    }

    if (!done && elapsed < kHoldLimit) {
        return true;
    }

    SPDLOG_INFO("scene prefetch: {} files, {} ms{}", sLoads.size(),
                std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count(),
                done ? "" : " (hold limit reached)");
    sActive = false;
    sLoads.clear();
    return false;
}

extern "C" void ScenePrefetch_UnloadAlt(void) {
    sStarted = false;
    auto resourceManager = Ship::Context::GetRawInstance()->GetResourceManager();
    auto files = resourceManager->GetArchiveManager()->ListFiles("alt/*");
    for (const auto& file : *files) {
        if (IsResident(file)) {
            auto resource = resourceManager->GetCachedResource(file, true);
            if (resource != nullptr &&
                resource->GetInitData()->Type == static_cast<uint32_t>(Fast::ResourceType::Texture)) {
                continue;
            }
        }
        resourceManager->UnloadResource(file);
        if (file.ends_with(".meta")) {
            resourceManager->UnloadResource(file.substr(0, file.size() - 5));
        }
    }
}
