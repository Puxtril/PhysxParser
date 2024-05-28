#include "PhysxReader.h"

using namespace Physx;

PhysxReader::PhysxReader(const std::filesystem::path& filepath)
    : m_managedReader(true)
{
    m_reader = new BinaryReader::BinaryReaderFile(filepath.string());
}

PhysxReader::PhysxReader(const std::string& filepath)
    : m_managedReader(true)
{
    m_reader = new BinaryReader::BinaryReaderFile(filepath);
}

PhysxReader::PhysxReader(std::vector<uint8_t>&& data)
    : m_managedReader(true)
{
    m_reader = new BinaryReader::BinaryReaderBuffered(std::move(data));
}

PhysxReader::PhysxReader(BinaryReader::BinaryReader* reader)
    : m_managedReader(false)
{
    m_reader = reader;
}

PhysxReader::~PhysxReader()
{
    if (m_managedReader)
        delete m_reader;
}

PhysxReader&
PhysxReader::seek(std::streamoff offset, std::ios_base::seekdir way)
{
    m_reader->seek(offset, way);
    return *this;
}

size_t
PhysxReader::tell()
{
    return m_reader->tell();
}

uint8_t
PhysxReader::readByte()
{
    return m_reader->readUInt8();
}

int16_t
PhysxReader::readInt16(bool isLittle)
{
    if (isLittle)
        return m_reader->readInt16();
    return m_reader->readInt16BE();
}

uint16_t
PhysxReader::readUInt16(bool isLittle)
{
    if (isLittle)
        return m_reader->readUInt16();
    return m_reader->readUInt16BE();
}

uint32_t
PhysxReader::readUInt32(bool isLittle)
{
    if (isLittle)
        return m_reader->readUInt32();
    return m_reader->readUInt32BE();
}

float
PhysxReader::readFloat(bool isLittle)
{
    if (isLittle)
        return m_reader->readFloat();

    // Weird but ok
    uint32_t tmp = m_reader->readUInt32BE();
    float flipped = *reinterpret_cast<float*>(&tmp);
    return flipped;
}

void
PhysxReader::readBytes(uint8_t* data, size_t length)
{
    m_reader->readUInt8Array(data, length);
}

void
PhysxReader::readInt16Array(int16_t* data, size_t length, bool isLittle)
{
    for (size_t i = 0; i < length; i++)
    {
        data[i] = readInt16(isLittle);
    }
}

void
PhysxReader::readUInt16Array(uint16_t* data, size_t length, bool isLittle)
{
    for (size_t i = 0; i < length; i++)
    {
        data[i] = readUInt16(isLittle);
    }
}

void
PhysxReader::readUInt32Array(uint32_t* data, size_t length, bool isLittle)
{
    for (size_t i = 0; i < length; i++)
    {
        data[i] = readUInt32(isLittle);
    }
}

void
PhysxReader::readFloatArray(float* data, size_t length, bool isLittle)
{
    for (size_t i = 0; i < length; i++)
    {
        data[i] = readFloat(isLittle);
    }
}