#pragma once

#include <cstdint>

// Shader combinations recorded from instrumented device runs (title, attract,
// file select, early gameplay). Compiled at boot so first-launch pipeline
// compilation does not stall the draw thread mid-scene.
static const uint64_t kSohShaderPrewarmList[][2] = {
    { 0x1080108ULL, 0xfffe0001ULL },          { 0x1080108ULL, 0xfffe0021ULL },
    { 0x108010cULL, 0xfffe0001ULL },          { 0x1081000ULL, 0xfffe0001ULL },
    { 0x1081000ULL, 0xfffe0021ULL },          { 0x1082821ULL, 0xfffe0001ULL },
    { 0x1082821ULL, 0xfffe0021ULL },          { 0x8012821ULL, 0xfffe0001ULL },
    { 0x8012821ULL, 0xfffe0021ULL },          { 0x10001000ULL, 0xfffe0000ULL },
    { 0x10001000ULL, 0xfffe0001ULL },         { 0x10001000ULL, 0xfffe0020ULL },
    { 0x10001000ULL, 0xfffe0021ULL },         { 0x10001000ULL, 0xfffe1021ULL },
    { 0x80008000ULL, 0xfffe0320ULL },         { 0x10d020d80000108ULL, 0xfffe0017ULL },
    { 0x10d020dc000818aULL, 0xfffe0013ULL },  { 0x10d0d0280000108ULL, 0xfffe0012ULL },
    { 0x10d0d0280000108ULL, 0xfffe0017ULL },  { 0x10d3d318000821aULL, 0xfffe0031ULL },
    { 0x20d020d01080108ULL, 0xfffe0011ULL },  { 0x20d3d3181ca821aULL, 0xfffe0031ULL },
    { 0x20d3d3181ca821aULL, 0xfffe0311ULL },  { 0x20d3d32818a818aULL, 0xfffe0011ULL },
    { 0x20d3d32818a818aULL, 0xfffe0031ULL },  { 0x1000020d00000108ULL, 0xfffe0012ULL },
    { 0xc0003d3100008218ULL, 0xfffe0010ULL }, { 0xd000020d10000108ULL, 0xfffe0012ULL },
    { 0xd000020d80000108ULL, 0xfffe0013ULL }, { 0xd000020d80000108ULL, 0xfffe0017ULL },
    { 0xd000020d80000108ULL, 0xfffe1017ULL }, { 0xd000020dc0000108ULL, 0xfffe0012ULL },
    { 0xd000020dc0000108ULL, 0xfffe0017ULL }, { 0xd000020dc0000108ULL, 0xfffe0112ULL },
    { 0xd000020dc0000108ULL, 0xfffe0212ULL }, { 0xd000020dc0000108ULL, 0xfffe0312ULL },
    { 0xd000020dc0000108ULL, 0xfffe0317ULL }, { 0xd000020dc000818aULL, 0xfffe0012ULL },
    { 0xd000020dc000818cULL, 0xfffe0012ULL }, { 0xd000030d08012821ULL, 0xfffe0013ULL },
    { 0xd0000d0280000108ULL, 0xfffe0012ULL }, { 0xd0000d0280000108ULL, 0xfffe0013ULL },
    { 0xd0000d0280000108ULL, 0xfffe0017ULL }, { 0xd0000d0280000108ULL, 0xfffe0112ULL },
    { 0xd0000d0280000108ULL, 0xfffe0312ULL }, { 0xd0003d3101088218ULL, 0xfffe0331ULL },
    { 0xd000d00001080108ULL, 0xfffe0013ULL }, { 0xd000d00001081000ULL, 0xfffe0013ULL },
    { 0xd000d00001082821ULL, 0xfffe0011ULL }, { 0xd000d00002010201ULL, 0xfffe0012ULL },
    { 0xd000d00008012821ULL, 0xfffe0013ULL }, { 0xd000d00010000108ULL, 0xfffe0012ULL },
    { 0xd000d00010000201ULL, 0xfffe0012ULL }, { 0xd000d000818a818aULL, 0xfffe0010ULL },
    { 0xd000d000c0000108ULL, 0xfffe0012ULL }, { 0xd000d000c0000201ULL, 0xfffe0012ULL },
    { 0xd000d000c0001000ULL, 0xfffe0012ULL }, { 0xd20a020d01080108ULL, 0xfffe0013ULL },
    { 0xd20a020d01080108ULL, 0xfffe0017ULL },
};
