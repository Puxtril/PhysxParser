#pragma once

#include "BinaryReaderBuffered.h"
#include "BinaryReaderFile.h"

#include <filesystem>
#include <string>
#include <vector>

namespace Physx
{
    class PhysxReader
    {
        BinaryReader::BinaryReader* m_reader;
        bool m_managedReader;

    public:
        PhysxReader(const std::filesystem::path& filepath);
        PhysxReader(const std::string& filepath);
        PhysxReader(std::vector<uint8_t>&& data);
        PhysxReader(BinaryReader::BinaryReader* reader);
        virtual ~PhysxReader();

        PhysxReader& seek(std::streamoff offset, std::ios_base::seekdir way);
        size_t tell();

        uint8_t readByte();
        int16_t readInt16(bool isLittle = 1);
        uint16_t readUInt16(bool isLittle = 1);
        uint32_t readUInt32(bool isLittle = 1);
        float readFloat(bool isLittle = 1);

        void readBytes(uint8_t* data, size_t length);
        void readInt16Array(int16_t* data, size_t length, bool isLittle = 1);
        void readUInt16Array(uint16_t* data, size_t length, bool isLittle = 1);
        void readUInt32Array(uint32_t* data, size_t length, bool isLittle = 1);
        void readFloatArray(float* data, size_t length, bool isLittle = 1);
    };
};