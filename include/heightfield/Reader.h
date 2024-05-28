#pragma once

#include "PhysxHeader.h"
#include "PhysxReader.h"
#include "heightfield/Structs.h"

namespace Physx::HeightFieldReader
{
    HeightFieldHeader readHeader(PhysxReader& reader, const PhysxHeader& physxHeader);
    std::vector<HeightFieldSample> readSamples(PhysxReader& reader, const PhysxHeader& physxHeader, const HeightFieldHeader& hfHeader);
    // Currently this ignores the material index
    HeightFieldMesh convertToMesh(const HeightFieldHeader& header, const std::vector<HeightFieldSample>& samples);
    HeightFieldMeshSplit convertToMeshSeparateMaterials(const HeightFieldHeader& header, const std::vector<HeightFieldSample>& samples);

    // Counts the amount of indices needed to generate the mesh
    // Basically, this filters out holes (material ID 127)
    size_t __getIndexCount(const std::vector<HeightFieldSample>& samples);

    // Same as above, but on a per-material basis
    std::vector<size_t> __getIndexCountPerMaterial(const std::vector<HeightFieldSample>& samples);
}