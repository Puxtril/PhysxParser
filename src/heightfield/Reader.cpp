#include "heightfield/Reader.h"

Physx::HeightFieldHeader
Physx::HeightFieldReader::readHeader(PhysxReader& reader, const PhysxHeader& physxHeader)
{
    HeightFieldHeader hfHeader;

    hfHeader.rowCount = reader.readUInt32(physxHeader.isLittle);
    hfHeader.columnCount = reader.readUInt32(physxHeader.isLittle);
    
    if (physxHeader.version > 1)
    {
        hfHeader.rowLimit = reader.readUInt32(physxHeader.isLittle);
        hfHeader.columnLimit = reader.readUInt32(physxHeader.isLittle);
        hfHeader.nbColumns = reader.readUInt32(physxHeader.isLittle);
    }
    else
    {
        hfHeader.rowLimit = static_cast<uint32_t>(reader.readFloat(physxHeader.isLittle));
        hfHeader.columnLimit = static_cast<uint32_t>(reader.readFloat(physxHeader.isLittle));
        hfHeader.nbColumns = static_cast<uint32_t>(reader.readFloat(physxHeader.isLittle));
    }

    hfHeader.thickness = reader.readFloat(physxHeader.isLittle);
    hfHeader.convexEdgeThreshold = reader.readFloat(physxHeader.isLittle);

    uint16_t noBoundaryEdgeInt = reader.readUInt16(physxHeader.isLittle);
    hfHeader.noBoundaryEdges = noBoundaryEdgeInt > 0 ? true : false;

    // There is only 1 sample format: S16_TM
    // Just check
    hfHeader.heightFieldFormat = reader.readUInt32(physxHeader.isLittle);
    if (hfHeader.heightFieldFormat != 1)
        throw std::runtime_error("HeightFieldFormat != 1: "  + std::to_string(hfHeader.heightFieldFormat));

    reader.readFloatArray(&hfHeader.minBounds[0], 3, physxHeader.isLittle);
    reader.readFloatArray(&hfHeader.maxBounds[0], 3, physxHeader.isLittle);

    hfHeader.sampleStride = reader.readUInt32(physxHeader.isLittle);
    hfHeader.sampleCount = reader.readUInt32(physxHeader.isLittle);
    hfHeader.sampleMinHeight = reader.readFloat(physxHeader.isLittle);
    hfHeader.sampleMaxHeight = reader.readFloat(physxHeader.isLittle);

    return hfHeader;
}

std::vector<Physx::HeightFieldSample>
Physx::HeightFieldReader::readSamples(PhysxReader& reader, const PhysxHeader& physxHeader, const HeightFieldHeader& hfHeader)
{
    std::vector<HeightFieldSample> samples(hfHeader.sampleCount);

    for (uint32_t i = 0; i < hfHeader.sampleCount; i++)
    {
        HeightFieldSample& curSample = samples[i];

        curSample.height = reader.readInt16(physxHeader.isLittle);

        uint8_t materialIndex0 = reader.readByte();
        curSample.tesselated = (materialIndex0 & 0x80) > 0 ? true : false;
        curSample.material0 = materialIndex0 & 0x7F;

        uint8_t materialIndex1 = reader.readByte();
        curSample.reserved = (materialIndex1 & 0x80) > 0 ? true : false;
        curSample.material1 = materialIndex1 & 0x7F;

        size_t customDataLen = hfHeader.sampleStride - 4;
        reader.seek(customDataLen, std::ios::cur);
    }

    return samples;
}

Physx::HeightFieldMesh
Physx::HeightFieldReader::convertToMesh(const HeightFieldHeader& header, const std::vector<HeightFieldSample>& samples)
{
    const uint32_t verticesPerRow = header.columnCount;
    const uint32_t verticesPerColumn = header.rowCount;

    HeightFieldMesh mesh;
    const size_t triangleCount = __getTriangleCount(samples);
    const size_t vertexCount = triangleCount * 3;

    mesh.materials.resize(triangleCount);
    mesh.vertexPositions.resize(vertexCount);

    uint32_t vertCursor = 0;
    uint32_t materialCursor = 0;
    for (uint32_t i = 0; i < header.sampleCount; i++)
    {
        const uint32_t curColumn = i % header.columnCount;
        const uint32_t curRow = i / header.rowCount;

        const HeightFieldSample& curSample = samples[i];

        // Edge of mesh
        if (curColumn == verticesPerColumn - 1 || curRow == verticesPerRow - 1)
            continue;

        if (curSample.tesselated)
        {
            if (curSample.material0 != 127)
            {
                mesh.vertexPositions[vertCursor++] = {(float)(curColumn    ), (float)samples[i                     ].height, (float)(curRow    )};
                mesh.vertexPositions[vertCursor++] = {(float)(curColumn    ), (float)samples[i + verticesPerRow    ].height, (float)(curRow + 1)};
                mesh.vertexPositions[vertCursor++] = {(float)(curColumn + 1), (float)samples[i + verticesPerRow + 1].height, (float)(curRow + 1)};
                mesh.materials[materialCursor++] = curSample.material0;
            }
            if (curSample.material1 != 127)
            {
                mesh.vertexPositions[vertCursor++] = {(float)(curColumn    ), (float)samples[i                     ].height, (float)(curRow    )};
                mesh.vertexPositions[vertCursor++] = {(float)(curColumn + 1), (float)samples[i + verticesPerRow + 1].height, (float)(curRow + 1)};
                mesh.vertexPositions[vertCursor++] = {(float)(curColumn + 1), (float)samples[i + 1                 ].height, (float)(curRow    )};
                mesh.materials[materialCursor++] = curSample.material1;
            }
        }
        else
        {
            if (curSample.material0 != 127)
            {
                mesh.vertexPositions[vertCursor++] = {(float)(curColumn + 1), (float)samples[i + 1                 ].height, (float)(curRow    )};
                mesh.vertexPositions[vertCursor++] = {(float)(curColumn    ), (float)samples[i                     ].height, (float)(curRow    )};
                mesh.vertexPositions[vertCursor++] = {(float)(curColumn    ), (float)samples[i + verticesPerRow    ].height, (float)(curRow + 1)};
                mesh.materials[materialCursor++] = curSample.material0;
            }
            if (curSample.material1 != 127)
            {
                mesh.vertexPositions[vertCursor++] = {(float)(curColumn    ), (float)samples[i + verticesPerRow    ].height, (float)(curRow + 1)};
                mesh.vertexPositions[vertCursor++] = {(float)(curColumn + 1), (float)samples[i + verticesPerRow + 1].height, (float)(curRow + 1)};
                mesh.vertexPositions[vertCursor++] = {(float)(curColumn + 1), (float)samples[i + 1                 ].height, (float)(curRow    )};
                mesh.materials[materialCursor++] = curSample.material1;
            }
        }
    }

    return mesh;
}

size_t
Physx::HeightFieldReader::__getTriangleCount(const std::vector<HeightFieldSample>& samples)
{
    size_t triangleCount = 0;

    for (uint32_t i = 0; i < samples.size(); i++)
    {
        if (samples[i].material0 != 127)
            triangleCount++;
        if (samples[i].material1 != 127)
            triangleCount++;
    }

    return triangleCount;
}

std::vector<size_t>
Physx::HeightFieldReader::__getIndexCountPerMaterial(const std::vector<HeightFieldSample>& samples)
{
    std::vector<size_t> indexCounts(127);

    for (uint32_t i = 0; i < samples.size(); i++)
    {
        if (samples[i].material0 != 127)
            indexCounts[samples[i].material0] += 3;
        if (samples[i].material1 != 127)
            indexCounts[samples[i].material1] += 3;
    }

    return indexCounts;
}