#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct PlayState;

void ScenePrefetch_Start(struct PlayState* play);
void ScenePrefetch_Room(int roomNum);
bool ScenePrefetch_Hold(void);
void ScenePrefetch_UnloadAlt(void);
void ScenePrefetch_Sky(struct PlayState* play);
int Environment_GetSkyboxPrefetch(struct PlayState* play, int ahead, const char** paths, int max);

#ifdef __cplusplus
}
#endif
