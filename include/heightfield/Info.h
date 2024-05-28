#pragma once

#include <vector>
#include <array>
#include <cstdint>

namespace Physx
{
    /**
    For more info: https://docs.nvidia.com/gameworks/content/gameworkslibrary/physx/guide/Manual/Geometry.html#height-fields

    Sources from Physx Repo:
        - PxHeightFieldSample.h
        - PxHeightFieldFlag.h
        - GuHeightField.h
        - GuHeightField.cpp
    **/ 
    const inline std::array<char, 4> HEIGHTFIELD_FOURCC = { 'H', 'F', 'H', 'F' };
    const inline std::vector<uint32_t> HEIGHTFIELD_VERSIONS = { 1, 2, 3 }; 
}