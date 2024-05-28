#pragma once

#include "PhysxReader.h"

#include <array>

namespace Physx
{
    struct PhysxHeader
    {
        std::array<char, 3> magic;
        bool isLittle;
        std::array<char, 4> fourCC;
        uint32_t version;
    };

    PhysxHeader readHeader(PhysxReader& reader);
    void readHeader(PhysxReader& reader, PhysxHeader& header);

    bool verifyHeader(PhysxHeader& header);
    bool verifyHeader(PhysxHeader& header, const std::string& fourCC);
    bool verifyHeader(PhysxHeader& header, const std::string& fourCC, uint32_t version);
}