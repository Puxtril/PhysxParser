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
    const size_t vertexCount = verticesPerColumn * verticesPerRow;
    const size_t indexCount = __getIndexCount(samples);

    HeightFieldMesh mesh;
    mesh.verts.resize(vertexCount);
    mesh.indices.resize(indexCount);

    // Create vertices
    for (uint32_t i = 0; i < vertexCount; i++)
    {
        const uint32_t curColumn = i % verticesPerRow;
        const uint32_t curRow = i / verticesPerColumn;

        // I think this is the default value?
        // Could be scaled by header.maxBounds and header.minBounds
        mesh.verts[i][0] = (float)curColumn;
        mesh.verts[i][1] = samples[i].height;
        mesh.verts[i][2] = (float)curRow;
    }

    // Create indices
    uint32_t indexCursor = 0;
    for (uint32_t i = 0; i < header.sampleCount; i++)
    {
        const uint32_t curColumn = i % header.columnCount;
        const uint32_t curRow = i / header.rowCount;
        const uint32_t curVertex = verticesPerRow * curRow + curColumn;
        const uint32_t belowCurVertex = verticesPerRow * (curRow + 1) + curColumn;

        const HeightFieldSample& curSample = samples[i];

        // Edge of mesh
        if (curColumn == verticesPerColumn - 1 || curRow == verticesPerRow - 1)
            continue;

        // Indices generation
        if (curSample.tesselated)
        {
            if (curSample.material0 != 127)
            {
                mesh.indices[indexCursor++] = curVertex;
                mesh.indices[indexCursor++] = belowCurVertex;
                mesh.indices[indexCursor++] = belowCurVertex + 1;
            }
            if (curSample.material1 != 127)
            {
                mesh.indices[indexCursor++] = curVertex;
                mesh.indices[indexCursor++] = belowCurVertex + 1;
                mesh.indices[indexCursor++] = curVertex + 1;
            }
        }
        else
        {
            if (curSample.material0 != 127)
            {
                mesh.indices[indexCursor++] = curVertex + 1;
                mesh.indices[indexCursor++] = curVertex;
                mesh.indices[indexCursor++] = belowCurVertex;
            }
            if (curSample.material1 != 127)
            {
                mesh.indices[indexCursor++] = belowCurVertex;
                mesh.indices[indexCursor++] = belowCurVertex + 1;
                mesh.indices[indexCursor++] = curVertex + 1;
            }
        }
    }

    return mesh;
}

Physx::HeightFieldMeshSplit
Physx::HeightFieldReader::convertToMeshSeparateMaterials(const HeightFieldHeader& header, const std::vector<HeightFieldSample>& samples)
{
    const uint32_t verticesPerRow = header.columnCount;
    const uint32_t verticesPerColumn = header.rowCount;
    const size_t vertexCount = verticesPerColumn * verticesPerRow;

    HeightFieldMeshSplit mesh;
    mesh.verts.resize(vertexCount);

    std::vector<size_t> indexCounts = __getIndexCountPerMaterial(samples);
    mesh.indexArrays.resize(indexCounts.size());
    for (size_t i = 0; i < indexCounts.size(); i++)
        mesh.indexArrays[i].resize(indexCounts[i]);

    // Create vertices
    for (uint32_t i = 0; i < vertexCount; i++)
    {
        const uint32_t curColumn = i % verticesPerRow;
        const uint32_t curRow = i / verticesPerColumn;

        // I think this is the default value?
        // Could be scaled by header.maxBounds and header.minBounds
        mesh.verts[i][0] = (float)curColumn;
        mesh.verts[i][1] = samples[i].height;
        mesh.verts[i][2] = (float)curRow;
    }

    std::vector<size_t> materialIndexArrayCursor(indexCounts.size());

    // Create indices
    for (uint32_t i = 0; i < header.sampleCount; i++)
    {
        const uint32_t curColumn = i % header.columnCount;
        const uint32_t curRow = i / header.rowCount;
        const uint32_t curVertex = verticesPerRow * curRow + curColumn;
        const uint32_t belowCurVertex = verticesPerRow * (curRow + 1) + curColumn;

        const HeightFieldSample& curSample = samples[i];

        // Edge of mesh
        if (curColumn == verticesPerColumn - 1 || curRow == verticesPerRow - 1)
            continue;

        if (curSample.tesselated)
        {
            if (curSample.material0 != 127)
            {
                const uint8_t& mat = curSample.material0;
                mesh.indexArrays[mat][materialIndexArrayCursor[mat]++] = curVertex;
                mesh.indexArrays[mat][materialIndexArrayCursor[mat]++] = belowCurVertex;
                mesh.indexArrays[mat][materialIndexArrayCursor[mat]++] = belowCurVertex + 1;
            }
            if (curSample.material1 != 127)
            {
                const uint8_t& mat = curSample.material1;
                mesh.indexArrays[mat][materialIndexArrayCursor[mat]++] = curVertex;
                mesh.indexArrays[mat][materialIndexArrayCursor[mat]++] = belowCurVertex + 1;
                mesh.indexArrays[mat][materialIndexArrayCursor[mat]++] = curVertex + 1;
            }
        }
        else
        {
            if (curSample.material0 != 127)
            {
                const uint8_t& mat = curSample.material0;
                mesh.indexArrays[mat][materialIndexArrayCursor[mat]++] = curVertex + 1;
                mesh.indexArrays[mat][materialIndexArrayCursor[mat]++] = curVertex;
                mesh.indexArrays[mat][materialIndexArrayCursor[mat]++] = belowCurVertex;
            }
            if (curSample.material1 != 127)
            {
                const uint8_t& mat = curSample.material1;
                mesh.indexArrays[mat][materialIndexArrayCursor[mat]++] = belowCurVertex;
                mesh.indexArrays[mat][materialIndexArrayCursor[mat]++] = belowCurVertex + 1;
                mesh.indexArrays[mat][materialIndexArrayCursor[mat]++] = curVertex + 1;
            }
        }
    }

    return mesh;
}

size_t
Physx::HeightFieldReader::__getIndexCount(const std::vector<HeightFieldSample>& samples)
{
    size_t indexCount = 0;

    for (uint32_t i = 0; i < samples.size(); i++)
    {
        if (samples[i].material0 != 127)
            indexCount += 3;
        if (samples[i].material1 != 127)
            indexCount += 3;
    }

    return indexCount;
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