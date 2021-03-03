#include "Utils/ByteBuffer.h"

FByteBuffer::FByteBuffer(const uint8* InBuffer, int32 InOffset, int32 InLen, bool bInTransferOwnerShip)
    : bLittleEndian(false),
    Version(0),
    Buffer(InBuffer),
    Offset(InOffset),
    Length(InLen),
    Position(0),
    bOwnsBuffer(bInTransferOwnerShip)
{
}

FByteBuffer::~FByteBuffer()
{
    if (bOwnsBuffer && Buffer != nullptr)
        FMemory::Free((void *)Buffer);
}

int32 FByteBuffer::GetBytesAvailable() const
{
    return Length - Position;
}

int8 FByteBuffer::ReadByte()
{
    signed char val = Buffer[Offset + Position];
    if (val > 127)
        val = val - 255;
    Position += 1;
    return val;
}

uint8 FByteBuffer::ReadUbyte()
{
    unsigned char val = Buffer[Offset + Position];
    Position += 1;
    return val;
}

bool FByteBuffer::ReadBool()
{
    return ReadByte() == 1;
}

int16 FByteBuffer::ReadShort()
{
    int32 startIndex = Offset + Position;
    Position += 2;
    uint8* pbyte = (uint8*)(Buffer + startIndex);
    if (bLittleEndian)
        return (int16)((*pbyte) | (*(pbyte + 1) << 8));
    else
        return (int16)((*pbyte << 8) | (*(pbyte + 1)));
}

uint16 FByteBuffer::ReadUshort()
{
    return (uint16)ReadShort();
}

int32 FByteBuffer::ReadInt()
{
    int32 startIndex = Offset + Position;
    Position += 4;
    uint8* pbyte = (uint8*)(Buffer + startIndex);
    if (bLittleEndian)
        return (*pbyte) | (*(pbyte + 1) << 8) | (*(pbyte + 2) << 16) | (*(pbyte + 3) << 24);
    else
        return (*pbyte << 24) | (*(pbyte + 1) << 16) | (*(pbyte + 2) << 8) | (*(pbyte + 3));
}

uint32 FByteBuffer::ReadUint()
{
    return (uint32)ReadInt();
}

float FByteBuffer::ReadFloat()
{
    int32 val = ReadInt();
    return *(float*)&val;
}

FString FByteBuffer::ReadString()
{
    int32 len = ReadUshort();
    return ReadString(len);
}

FString FByteBuffer::ReadString(int32 InLen)
{
    uint8* value = (uint8*)FMemory::Malloc(InLen + 1);

    value[InLen] = '\0';
    FMemory::Memcpy(value, Buffer + Position, InLen);
    Position += InLen;

    FString str = UTF8_TO_TCHAR(value);
    FMemory::Free(value);

    return str;
}

const FString& FByteBuffer::ReadS()
{
    uint16 index = ReadUshort();
    if (index == 65534 || index == 65533)
        return G_EMPTY_STRING;
    else
        return (*StringTable)[index];
}

bool FByteBuffer::ReadS(FString& OutString)
{
    uint16 index = ReadUshort();
    if (index == 65534) //null
        return false;
    else if (index == 65533)
    {
        OutString.Reset();
        return true;
    }
    else
    {
        OutString = (*StringTable)[index];
        return true;
    }
}

const FString* FByteBuffer::ReadSP()
{
    uint16 index = ReadUshort();
    if (index == 65534) //null
        return nullptr;
    else if (index == 65533)
        return &G_EMPTY_STRING;
    else
        return &(*StringTable)[index];
}

void FByteBuffer::ReadSArray(TArray<FString>& OutArray, int32 InCount)
{
    for (int32 i = 0; i < InCount; i++)
        OutArray.Push(ReadS());
}

void FByteBuffer::WriteS(const FString& InString)
{
    uint16 index = ReadUshort();
    if (index != 65534 && index != 65533)
        (*StringTable)[index] = InString;
}

FColor FByteBuffer::ReadColor()
{
    int32 startIndex = Offset + Position;
    uint8 r = Buffer[startIndex];
    uint8 g = Buffer[startIndex + 1];
    uint8 b = Buffer[startIndex + 2];
    uint8 a = Buffer[startIndex + 3];
    Position += 4;

    return FColor(r, g, b, a);
}

TSharedPtr<FByteBuffer> FByteBuffer::ReadBuffer(bool bCloneBuffer)
{
    int32 count = ReadInt();
    FByteBuffer* ba;
    if (bCloneBuffer)
    {
        uint8* p = (uint8*)FMemory::Malloc(count);
        memcpy(p, Buffer + Position, count);
        ba = new FByteBuffer(p, 0, count, true);
    }
    else
        ba = new FByteBuffer(Buffer, Position, count, false);
    ba->StringTable = StringTable;
    ba->Version = Version;
    Position += count;
    return MakeShareable(ba);
}

bool FByteBuffer::Seek(int32 IndexTablePos, int32 BlockIndex)
{
    int32 tmp = Position;
    Position = IndexTablePos;
    int32 segCount = Buffer[Offset + Position++];
    if (BlockIndex < segCount)
    {
        bool useShort = Buffer[Offset + Position++] == 1;
        int32 newPos;
        if (useShort)
        {
            Position += 2 * BlockIndex;
            newPos = ReadShort();
        }
        else
        {
            Position += 4 * BlockIndex;
            newPos = ReadInt();
        }

        if (newPos > 0)
        {
            Position = IndexTablePos + newPos;
            return true;
        }
        else
        {
            Position = tmp;
            return false;
        }
    }
    else
    {
        Position = tmp;
        return false;
    }
}
