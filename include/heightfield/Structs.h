#pragma once

#include <cstdint>
#include <array>
#include <vector>

namespace Physx
{
    struct HeightFieldHeader
    {
        uint32_t rowCount;
        uint32_t columnCount;
        uint32_t rowLimit;
        uint32_t columnLimit;
        uint32_t nbColumns;
        // Unused
        float thickness;
        float convexEdgeThreshold;
        // Disable collisions with height field with boundary edges
        // Raise this flag if several terrain patches are going to be placed adjacent to each other, 
		//   to avoid a bump when sliding across.
        bool noBoundaryEdges;
        // Only 1 sample format: S16_TM (0x1)
        uint32_t heightFieldFormat;
        std::array<float, 3> minBounds;
        std::array<float, 3> maxBounds;
        uint32_t sampleStride;
        uint32_t sampleCount;
        // Scale the sample height
        float sampleMinHeight;
        float sampleMaxHeight;
    };

    /**
    Material0 and Material1 point to the two triangles in the current quad/sample
        - Stored in-file as uint8, but only 7 bits for material index
        - First bit of material0 is tesselation flag
        - First bit of material1 is reserved flag
        - A value of 0x7F (127) means the triangle is a hole.

    Tesselation flag = false
       +--+
     0 | /|
       |/ | 1
       +--+

    Tesselation flags = true;
       +--+
       |\ | 1
     0 | \|
       +--+
    **/
    struct HeightFieldSample
    {
        int16_t height;
        bool tesselated;
        bool reserved;
        uint8_t material0;
        uint8_t material1;
    };

    // Converted mesh structs
    struct HeightFieldMesh
    {
        std::vector<std::array<float, 3>> verts;
        std::vector<uint32_t> indices;
    };

    struct HeightFieldMeshSplit
    {
        std::vector<std::array<float, 3>> verts;
        std::vector<std::vector<uint32_t>> indexArrays;
    };
}