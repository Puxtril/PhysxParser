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

// This is definitely in need of optimization.
// I got this working with the first idea I had.
Physx::HeightFieldIndexedMesh
Physx::HeightFieldReader::convertToIndexedMesh(const HeightFieldHeader& header, const std::vector<HeightFieldSample>& samples)
{
    const uint32_t verticesPerRow = header.columnCount;
    const uint32_t verticesPerColumn = header.rowCount;

    HeightFieldIndexedMesh mesh;

    std::vector<bool> validVertices(header.sampleCount);
    size_t faceCount = 0;
    uint32_t vertexCount = 0;

    // Calculate the total valid vertices for the final mesh (Skip the holes).
    // We must go per-material to calculate the edges of materials.
    // Finding which vertices are valid, I've found the easiest method is to use a bitarray (vector of bools, because bitarray requires static allocation...)
    for (int iMaterial = 0; iMaterial < 256; iMaterial++)
    {
        if (iMaterial == 127)
            continue;

        std::fill_n(validVertices.begin(), validVertices.size(), false);

        // Find which vertices in the current material are valid
        for (uint32_t i = 0; i < header.sampleCount; i++)
        {
            const uint32_t curColumn = i % header.columnCount;
            const uint32_t curRow = i / header.rowCount;

            const HeightFieldSample& curSample = samples[i];

            // Edge of mesh
            if (curColumn == verticesPerColumn - 1 || curRow == verticesPerRow - 1)
                continue;

            const uint32_t curVertex = curRow * verticesPerRow + curColumn;
            if (curSample.tesselated)
            {
                if (curSample.material0 == iMaterial)
                {
                    validVertices[curVertex] = true;
                    validVertices[curVertex + verticesPerRow] = true;
                    validVertices[curVertex + verticesPerRow + 1] = true;
                    faceCount += 3;
                }
                if (curSample.material1 == iMaterial)
                {
                    validVertices[curVertex] = true;
                    validVertices[curVertex + verticesPerRow + 1] = true;
                    validVertices[curVertex + 1] = true;
                    faceCount += 3;
                }
            }
            else
            {
                if (curSample.material0 == iMaterial)
                {
                    validVertices[curVertex + 1] = true;
                    validVertices[curVertex] = true;
                    validVertices[curVertex + verticesPerRow] = true;
                    faceCount += 3;
                }
                if (curSample.material1 == iMaterial)
                {
                    validVertices[curVertex + verticesPerRow] = true;
                    validVertices[curVertex + verticesPerRow + 1] = true;
                    validVertices[curVertex + 1] = true;
                    faceCount += 3;
                }
            }
        }

        // Calculate total vertex count for this material, add to master total.
        for (int i = 0; i < header.sampleCount; i++)
        {
            if (validVertices[i])
                vertexCount++;
        }
    }

    // Use the calculated variables to declare our final size.
    mesh.vertexPositions.resize(vertexCount);
    mesh.materials.resize(vertexCount);
    mesh.indices.resize(faceCount);

    uint32_t vertexOffset = 0;
    uint32_t indexOffset = 0;

    // Generate the indices.
    for (int iMaterial = 0; iMaterial < 256; iMaterial++)
    {
        if (iMaterial == 127)
            continue;

        size_t curIndexCount = 0;

        // Yes, this is re-running the same code used above to calculate vertex counts.
        // I chose this over storing above calculations (more processing vs more RAM usage)
        std::fill_n(validVertices.begin(), validVertices.size(), false);
        for (uint32_t i = 0; i < header.sampleCount; i++)
        {
            const uint32_t curColumn = i % header.columnCount;
            const uint32_t curRow = i / header.rowCount;

            const HeightFieldSample& curSample = samples[i];

            // Edge of mesh
            if (curColumn == verticesPerColumn - 1 || curRow == verticesPerRow - 1)
                continue;

            const uint32_t curVertex = curRow * verticesPerRow + curColumn;
            if (curSample.tesselated)
            {
                if (curSample.material0 == iMaterial)
                {
                    validVertices[curVertex] = true;
                    validVertices[curVertex + verticesPerRow] = true;
                    validVertices[curVertex + verticesPerRow + 1] = true;
                    curIndexCount += 3;
                }
                if (curSample.material1 == iMaterial)
                {
                    validVertices[curVertex] = true;
                    validVertices[curVertex + verticesPerRow + 1] = true;
                    validVertices[curVertex + 1] = true;
                    curIndexCount += 3;
                }
            }
            else
            {
                if (curSample.material0 == iMaterial)
                {
                    validVertices[curVertex + 1] = true;
                    validVertices[curVertex] = true;
                    validVertices[curVertex + verticesPerRow] = true;
                    curIndexCount += 3;
                }
                if (curSample.material1 == iMaterial)
                {
                    validVertices[curVertex + verticesPerRow] = true;
                    validVertices[curVertex + verticesPerRow + 1] = true;
                    validVertices[curVertex + 1] = true;
                    curIndexCount += 3;
                }
            }
        }

        // Vertex count for this material.
        uint32_t curVertexCount = 0;
        for (int i = 0; i < header.sampleCount; i++)
        {
            if (validVertices[i])
                curVertexCount++;
        }

        if (curVertexCount == 0)
            continue;

        // Generate vertex positions and materials (vertex color).
        size_t vertexCursor = vertexOffset;
        for (int i = 0; i < header.sampleCount; i++)
        {
            if (!validVertices[i])
                continue;

            const uint32_t curColumn = i % header.columnCount;
            const uint32_t curRow = i / header.rowCount;
            const HeightFieldSample& curSample = samples[i];

            mesh.materials[vertexCursor] = iMaterial;
            mesh.vertexPositions[vertexCursor++] = {(float)(curColumn), (float)curSample.height, (float)(curRow)};
        }

        // Below we will iterate over samples, but we also must match the current sample with the current vertex.
        // Additionally, we must know where the vertex in the next *row* is.
        // After all, vertices will NOT line up with samples (because of holes).
        // Here, start the `vertCursorNextRow` at the correct position.
        uint32_t vertCursorNextRow = vertexOffset;
        for (uint32_t i = 0; i < verticesPerRow; i++)
        {
            if (validVertices[i])
                vertCursorNextRow += 1;
        }
        
        // Generate indicies.
        // Keeping careful track of the current vertex and next row vertex.
        uint32_t vertCursor = vertexOffset;
        size_t indexCursor = indexOffset;
        for (uint32_t iSample = 0; iSample < header.sampleCount; iSample++)
        {
            const uint32_t curColumn = iSample % header.columnCount;
            const uint32_t curRow = iSample / header.rowCount;

            const HeightFieldSample& curSample = samples[iSample];

            // Edge of mesh
            if (curColumn == verticesPerColumn - 1 || curRow == verticesPerRow - 1)
                continue;

            if (curSample.tesselated)
            {
                if (curSample.material0 == iMaterial)
                {
                    mesh.indices[indexCursor++] = vertCursor;
                    mesh.indices[indexCursor++] = vertCursorNextRow;
                    mesh.indices[indexCursor++] = vertCursorNextRow + 1;
                }
                if (curSample.material1 == iMaterial)
                {
                    mesh.indices[indexCursor++] = vertCursor;
                    mesh.indices[indexCursor++] = vertCursorNextRow + 1;
                    mesh.indices[indexCursor++] = vertCursor + 1;
                }
            }
            else
            {
                if (curSample.material0 == iMaterial)
                {
                    mesh.indices[indexCursor++] = vertCursor + 1;
                    mesh.indices[indexCursor++] = vertCursor;
                    mesh.indices[indexCursor++] = vertCursorNextRow;
                }
                if (curSample.material1 == iMaterial)
                {
                    mesh.indices[indexCursor++] = vertCursorNextRow;
                    mesh.indices[indexCursor++] = vertCursorNextRow + 1;
                    mesh.indices[indexCursor++] = vertCursor + 1;
                }
            }

            // Advance the 2 cursors.
            // If we hit the heightfield edge, skip that too.
            if (validVertices[iSample])
            {
                vertCursor++;
                if (curColumn == verticesPerRow - 2 && validVertices[iSample + 1])
                    vertCursor++;
            }

            if (validVertices[iSample + verticesPerRow])
            {
                vertCursorNextRow++;
                if (curColumn == verticesPerRow - 2 && validVertices[iSample + verticesPerRow + 1])
                    vertCursorNextRow++;
            }
        }

        indexOffset += curIndexCount;
        vertexOffset += curVertexCount;
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