#include "PhysxHeader.h"

Physx::PhysxHeader
Physx::readHeader(PhysxReader& reader)
{
    PhysxHeader header;
    readHeader(reader, header);
    return header;
}

void
Physx::readHeader(PhysxReader& reader, PhysxHeader& header)
{
    reader.readBytes((uint8_t*)&header.magic[0], 3);
    header.isLittle = reader.readByte();
    reader.readBytes((uint8_t*)&header.fourCC[0], 4);
    header.version = reader.readUInt32();
}

bool
Physx::verifyHeader(PhysxHeader& header)
{
    if (header.magic[0] != 'N' || header.magic[1] != 'X' || header.magic[2] != 'S')
        return false;

    return true;
}

bool
Physx::verifyHeader(PhysxHeader& header, const std::string& fourCC)
{
    if (!verifyHeader(header))
        return false;

    return strcmp(fourCC.c_str(), &header.fourCC[0]) == 0;
}

bool
Physx::verifyHeader(PhysxHeader& header, const std::string& fourCC, uint32_t version)
{
    if (!verifyHeader(header, fourCC))
        return false;

    return header.version != version;
}