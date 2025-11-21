#pragma once
#include <cstdint>
#include "../../Utils/MemoryUtil.h"

class Weather {
public:
    CLASS_MEMBER(float, rainLevel, 0x38);
    CLASS_MEMBER(float, lightningLevel, 0x44);
    CLASS_MEMBER(float, fogLevel, 0x4C);
};

namespace DimensionID {
    constexpr inline int32_t OVERWORLD = 0;
    constexpr inline int32_t NETHER = 1;
    constexpr inline int32_t END = 2;
    constexpr inline int32_t UNDEFINED = 3;
};

class Dimension {
public:
    CLASS_MEMBER(int32_t, id, 0x218);
    CLASS_MEMBER(class Weather*, Weather, 0x1B0);
    CLASS_MEMBER(__int64*, chunkSource, 0x1A0);

    struct HeightRange {
        int16_t mMinHeight;
        int16_t mMaxHeight;
    };

    int getDimensionID() {
        return MemoryUtil::CallVFunc<2, int>(this);
    }

    int getDefaultBiome() {
        return MemoryUtil::CallVFunc<22, int>(this);
    }

    bool isChunkKnown(int pos[2]);
};

