#pragma once

#include <simd/simd.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool HasQuad;
    simd_float4x4 ImmersiveFromQuad;
    float HalfWidth;
    float HalfHeight;
} SohVolumeFrame;

void SohVolumeStart(void* device, void* commandQueue, uint32_t width, uint32_t height);

void SohVolumeSetShutdownHandler(void (*handler)(void));

void SohVolumeStop(void);

void SohVolumeUpdate(SohVolumeFrame frame);

float SohVolumeAspect(void);

void SohVolumeNote(const char* text);

void SohVolumeOpenMenu(void);

void SohVolumePoint(float x, float y, bool pressed);

typedef struct {
    float MinX;
    float MinY;
    float MaxX;
    float MaxY;
    uint64_t Identifier;
} SohVolumeHoverRect;

size_t SohVolumeHoverRects(SohVolumeHoverRect* out, size_t max);

void SohVolumeSetScenePhase(int phase);

void SohVolumeSetStereo(bool stereo);

void* SohVolumeTexture(int eye);

void SohVolumeNoteHoverLayout(int rebuilt);

void SohVolumeNoteCopySkipped(void);

#ifdef __cplusplus
}
#endif
