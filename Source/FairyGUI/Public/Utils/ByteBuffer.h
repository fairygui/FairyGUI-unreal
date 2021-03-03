#pragma once

#include "CoreMinimal.h"
#include "FairyCommons.h"

class FAIRYGUI_API FByteBuffer
{
public:
    FByteBuffer(const uint8* InBuffer, int32 InOffset, int32 InLen, bool bInTransferOwnerShip);
    ~FByteBuffer();

    const uint8* GetBuffer() const { return Buffer; }

    int32 GetBytesAvailable() const;
    int32 GetLength() const { return Length; }

    int32 GetPos() const { return Position; }
    void SetPos(int32 InPos) { Position = InPos; }
    void Skip(int32 Count) { Position += Count; }

    int8 ReadByte();
    uint8 ReadUbyte();
    bool ReadBool();
    int16 ReadShort();
    uint16 ReadUshort();
    int32 ReadInt();
    uint32 ReadUint();
    float ReadFloat();
    FString ReadString();
    FString ReadString(int32 InLen);
    const FString& ReadS();
    void ReadSArray(TArray<FString>& OutArray, int32 InCount);
    bool ReadS(FString& OutString);
    const FString* ReadSP();
    void WriteS(const FString& InString);
    FColor ReadColor();
    TSharedPtr<FByteBuffer> ReadBuffer(bool bCloneBuffer);
    bool Seek(int32 IndexTablePos, int32 BlockIndex);

    bool bLittleEndian;
    int32 Version;
    TSharedPtr<TArray<FString>> StringTable;

private:
    const uint8* Buffer;
    int32 Offset;
    int32 Length;
    int32 Position;;
    bool bOwnsBuffer;
};
