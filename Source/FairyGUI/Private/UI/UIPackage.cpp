#include "UI/UIPackage.h"
#include "Sound/SoundBase.h"
#include "UIPackageAsset.h"
#include "UI/PackageItem.h"
#include "UI/GObject.h"
#include "Widgets/NTexture.h"
#include "Widgets/SMovieClip.h"
#include "Widgets/BitmapFont.h"
#include "Utils/ByteBuffer.h"
#include "UI/UIObjectFactory.h"

int32 UUIPackage::Constructing = 0;

struct FAtlasSprite
{
    FAtlasSprite() :
        Rect(ForceInit),
        OriginalSize(ForceInit),
        Offset(ForceInit),
        bRotated(false)
    {
    }
    TSharedPtr<FPackageItem> Atlas;
    FBox2D Rect;
    FVector2D OriginalSize;
    FVector2D Offset;
    bool bRotated;
};

const FString& UUIPackage::GetBranch()
{
    return UUIPackageStatic::Get().Branch;
}

void UUIPackage::SetBranch(const FString& InBranch)
{
    UUIPackageStatic::Get().Branch = InBranch;
    bool empty = InBranch.IsEmpty();
    for (auto& it : UUIPackageStatic::Get().PackageInstByID)
    {
        UUIPackage*& Pkg = it.Value;
        if (empty)
            Pkg->BranchIndex = -1;
        else if (Pkg->Branches.Num() > 0)
            Pkg->BranchIndex = Pkg->Branches.Find(InBranch);
    }
}

FString UUIPackage::GetVar(const FString& VarKey)
{
    FString* Value = UUIPackageStatic::Get().Vars.Find(VarKey);
    if (Value != nullptr)
        return *Value;
    else
        return G_EMPTY_STRING;
}

void UUIPackage::SetVar(const FString& VarKey, const FString& VarValue)
{
    UUIPackageStatic::Get().Vars.Add(VarKey, VarValue);
}

UUIPackage* UUIPackage::AddPackage(const TCHAR* InAssetPath, UObject* WorldContextObject)
{
    UUIPackageAsset* PackageAsset = Cast<UUIPackageAsset>(StaticLoadObject(UUIPackageAsset::StaticClass(), nullptr, InAssetPath));
    verifyf(PackageAsset != nullptr, TEXT("Asset not found %s"), InAssetPath);

    return AddPackage(PackageAsset, WorldContextObject);
}

UUIPackage* UUIPackage::AddPackage(UUIPackageAsset* InAsset, UObject* WorldContextObject)
{
    verifyf(WorldContextObject != nullptr, TEXT("Null WorldContextObject?"));

    UWorld* World = WorldContextObject->GetWorld();
    verifyf(World != nullptr, TEXT("Null World?"));
    verifyf(World->IsGameWorld(), TEXT("Not a Game World?"));

    UUIPackage* Pkg = UUIPackageStatic::Get().PackageInstByID.FindRef(InAsset->GetPathName());
    if (Pkg != nullptr)
    {
        if (Pkg->RefWorlds.Contains(World->GetUniqueID()))
        {
            UE_LOG(LogFairyGUI, Warning, TEXT("Package already addedd"));
        }
        else
            Pkg->RefWorlds.Add(World->GetUniqueID());
        return Pkg;
    }

    FByteBuffer Buffer(InAsset->Data.GetData(), 0, InAsset->Data.Num(), false);

    Pkg = NewObject<UUIPackage>();
    Pkg->RefWorlds.Add(World->GetUniqueID());
    Pkg->Asset = InAsset;
    Pkg->AssetPath = InAsset->GetPathName();
    Pkg->Load(&Buffer);

    UUIPackageStatic::Get().PackageList.Add(Pkg);
    UUIPackageStatic::Get().PackageInstByID.Add(Pkg->ID, Pkg);
    UUIPackageStatic::Get().PackageInstByID.Add(Pkg->AssetPath, Pkg);
    UUIPackageStatic::Get().PackageInstByName.Add(Pkg->Name, Pkg);

    return Pkg;
}

void UUIPackage::RemovePackage(const FString& IDOrName, UObject* WorldContextObject)
{
    verifyf(WorldContextObject != nullptr, TEXT("Null WorldContextObject?"));

    UUIPackage* Pkg = GetPackageByName(IDOrName);
    if (Pkg == nullptr)
        Pkg = GetPackageByID(IDOrName);

    if (Pkg != nullptr)
    {
        UWorld* World = WorldContextObject->GetWorld();
        verifyf(World != nullptr, TEXT("Null World?"));
        verifyf(World->IsGameWorld(), TEXT("Not a Game World?"));
        Pkg->RefWorlds.Remove(World->GetUniqueID());

        if (Pkg->RefWorlds.Num() > 0)
            return;

        UUIPackageStatic::Get().PackageList.Remove(Pkg);
        UUIPackageStatic::Get().PackageInstByID.Remove(Pkg->ID);
        UUIPackageStatic::Get().PackageInstByID.Remove(Pkg->AssetPath);
        UUIPackageStatic::Get().PackageInstByName.Remove(Pkg->Name);
    }
    else
        UE_LOG(LogFairyGUI, Error, TEXT("invalid package name or id: %s"), *IDOrName);
}

void UUIPackage::RemoveAllPackages()
{
    UUIPackageStatic::Get().PackageList.Reset();
    UUIPackageStatic::Get().PackageInstByID.Reset();
    UUIPackageStatic::Get().PackageInstByName.Reset();
}

UUIPackage* UUIPackage::GetPackageByID(const FString& PackageID)
{
    auto it = UUIPackageStatic::Get().PackageInstByID.Find(PackageID);
    if (it != nullptr)
        return *it;
    else
        return nullptr;
}

UUIPackage* UUIPackage::GetPackageByName(const FString& PackageName)
{
    auto it = UUIPackageStatic::Get().PackageInstByName.Find(PackageName);
    if (it != nullptr)
        return *it;
    else
        return nullptr;
}

UGObject* UUIPackage::CreateObject(const FString& PackageName, const FString& ResourceName, UObject* WorldContextObject, TSubclassOf<UGObject> ClassType)
{
    UUIPackage* pkg = UUIPackage::GetPackageByName(PackageName);
    if (pkg)
        return pkg->CreateObject(ResourceName, WorldContextObject);
    else
        return nullptr;
}

UGObject* UUIPackage::CreateObjectFromURL(const FString& URL, UObject* WorldContextObject, TSubclassOf<UGObject> ClassType)
{
    TSharedPtr<FPackageItem> pii = UUIPackage::GetItemByURL(URL);
    if (pii.IsValid())
        return pii->Owner->CreateObject(pii, WorldContextObject);
    else
        return nullptr;
}

FString UUIPackage::GetItemURL(const FString& PackageName, const FString& ResourceName)
{
    UUIPackage* pkg = GetPackageByName(PackageName);
    if (pkg != nullptr)
    {
        TSharedPtr<FPackageItem> pii = pkg->GetItemByName(ResourceName);
        if (pii.IsValid())
            return "ui://" + pkg->GetID() + pii->ID;
    }
    return "";
}

TSharedPtr<FPackageItem> UUIPackage::GetItemByURL(const FString& URL)
{
    if (URL.IsEmpty())
        return nullptr;

    int32 pos1;
    if (!URL.FindChar('/', pos1))
        return nullptr;

    int32 pos2 = URL.Find("/", ESearchCase::CaseSensitive, ESearchDir::FromStart, pos1 + 2);
    if (pos2 == -1) {
        if (URL.Len() > 13)
        {
            FString pkgId = URL.Mid(5, 8);
            UUIPackage* pkg = GetPackageByID(pkgId);
            if (pkg != nullptr)
            {
                FString srcId = URL.Mid(13);
                return pkg->GetItem(srcId);
            }
        }
    }
    else
    {
        FString pkgName = URL.Mid(pos1 + 2, pos2 - pos1 - 2);
        UUIPackage* pkg = GetPackageByName(pkgName);
        if (pkg != nullptr)
        {
            FString srcName = URL.Mid(pos2 + 1);
            return pkg->GetItemByName(srcName);
        }
    }

    return nullptr;
}

FString UUIPackage::NormalizeURL(const FString& URL)
{
    if (URL.IsEmpty())
        return URL;

    int32 pos1;
    if (!URL.FindChar('/', pos1))
        return URL;

    int32 pos2 = URL.Find("/", ESearchCase::CaseSensitive, ESearchDir::FromStart, pos1 + 2);
    if (pos2 == -1)
        return URL;
    else
    {
        FString pkgName = URL.Mid(pos1 + 2, pos2 - pos1 - 2);
        FString srcName = URL.Mid(pos2 + 1);
        return GetItemURL(pkgName, srcName);
    }
}

UUIPackage::UUIPackage()
{

}

UUIPackage::~UUIPackage()
{
    for (auto& it : Sprites)
        delete it.Value;
}

TSharedPtr<FPackageItem> UUIPackage::GetItem(const FString& ItemID) const
{
    auto it = ItemsByID.Find(ItemID);
    if (it != nullptr)
        return *it;
    else
        return nullptr;
}

TSharedPtr<FPackageItem> UUIPackage::GetItemByName(const FString& ResourceName)
{
    auto it = ItemsByName.Find(ResourceName);
    if (it != nullptr)
        return *it;
    else
        return nullptr;
}

UGObject* UUIPackage::CreateObject(const FString& ResourceName, UObject* WorldContextObject)
{
    TSharedPtr<FPackageItem> item = GetItemByName(ResourceName);
    verifyf(item.IsValid(), TEXT("FairyGUI: resource not found - %s in  %s"), *ResourceName, *Name);

    return CreateObject(item, WorldContextObject);
}

UGObject* UUIPackage::CreateObject(const TSharedPtr<FPackageItem>& Item, UObject* WorldContextObject)
{
    UGObject* g = FUIObjectFactory::NewObject(Item, WorldContextObject);
    if (g == nullptr)
        return nullptr;

    Constructing++;
    g->ConstructFromResource();
    Constructing--;
    return g;
}

void UUIPackage::RegisterFont(const FString& FontFace, UObject* Font)
{
    UUIPackageStatic::Get().Fonts.Add(FontFace, Font);
}

void UUIPackage::Load(FByteBuffer* Buffer)
{
    if (Buffer->ReadUint() != 0x46475549)
    {
        UE_LOG(LogFairyGUI, Error, TEXT("not valid package format in %d '%s'"), Buffer->ReadUint(), *AssetPath);
        return;
    }

    Buffer->Version = Buffer->ReadInt();
    bool ver2 = Buffer->Version >= 2;
    Buffer->ReadBool(); //compressed
    ID = Buffer->ReadString();
    Name = Buffer->ReadString();
    Buffer->Skip(20);
    int32 indexTablePos = Buffer->GetPos();
    int32 cnt;

    Buffer->Seek(indexTablePos, 4);

    cnt = Buffer->ReadInt();
    TArray<FString>* StringTable = new TArray<FString>();
    StringTable->SetNum(cnt, true);
    for (int32 i = 0; i < cnt; i++)
    {
        (*StringTable)[i] = Buffer->ReadString();
    }
    Buffer->StringTable = MakeShareable(StringTable);

    Buffer->Seek(indexTablePos, 0);
    cnt = Buffer->ReadShort();
    for (int32 i = 0; i < cnt; i++)
    {
        TMap<FString, FString> info;
        info.Add("id", Buffer->ReadS());
        info.Add("name", Buffer->ReadS());

        Dependencies.Push(info);
    }

    bool branchIncluded = false;
    if (ver2)
    {
        cnt = Buffer->ReadShort();
        if (cnt > 0)
        {
            Buffer->ReadSArray(Branches, cnt);
            if (!UUIPackageStatic::Get().Branch.IsEmpty())
                BranchIndex = Branches.Find(UUIPackageStatic::Get().Branch);
        }

        branchIncluded = cnt > 0;
    }

    Buffer->Seek(indexTablePos, 1);

    FString path = FPaths::GetPath(AssetPath);
    FString fileName = FPaths::GetBaseFilename(AssetPath);

    cnt = Buffer->ReadShort();
    for (int32 i = 0; i < cnt; i++)
    {
        int32 nextPos = Buffer->ReadInt();
        nextPos += Buffer->GetPos();

        TSharedPtr<FPackageItem> pii = MakeShared<FPackageItem>();
        pii->Owner = this;
        pii->Type = (EPackageItemType)Buffer->ReadByte();
        pii->ID = Buffer->ReadS();
        pii->Name = Buffer->ReadS();
        Buffer->Skip(2); //path
        pii->File = Buffer->ReadS();
        Buffer->ReadBool(); //exported
        pii->Size.X = Buffer->ReadInt();
        pii->Size.Y = Buffer->ReadInt();

        switch (pii->Type)
        {
        case EPackageItemType::Image:
        {
            pii->ObjectType = EObjectType::Image;
            int32 scaleOption = Buffer->ReadByte();
            if (scaleOption == 1)
            {
                FBox2D scale9Grid(ForceInit);
                scale9Grid.Min.X = Buffer->ReadInt();
                scale9Grid.Min.Y = Buffer->ReadInt();
                scale9Grid.Max.X = scale9Grid.Min.X + Buffer->ReadInt();
                scale9Grid.Max.Y = scale9Grid.Min.Y + Buffer->ReadInt();
                pii->Scale9Grid = scale9Grid;
                pii->TileGridIndice = Buffer->ReadInt();
            }
            else if (scaleOption == 2)
                pii->bScaleByTile = true;

            Buffer->ReadBool(); //smoothing
            break;
        }

        case EPackageItemType::MovieClip:
        {
            Buffer->ReadBool(); //smoothing
            pii->ObjectType = EObjectType::MovieClip;
            pii->RawData = Buffer->ReadBuffer(false);
            break;
        }

        case EPackageItemType::Font:
        {
            pii->RawData = Buffer->ReadBuffer(false);
            break;
        }

        case EPackageItemType::Component:
        {
            int32 extension = Buffer->ReadByte();
            if (extension > 0)
                pii->ObjectType = (EObjectType)extension;
            else
                pii->ObjectType = EObjectType::Component;
            pii->RawData = Buffer->ReadBuffer(false);

            FUIObjectFactory::ResolvePackageItemExtension(pii);
            break;
        }

        case EPackageItemType::Atlas:
        case EPackageItemType::Sound:
        case EPackageItemType::Misc:
        {
            FString file = fileName + "_" + FPaths::GetBaseFilename(pii->File);
            pii->File = path + "/" + file + "." + file;
            break;
        }

        case EPackageItemType::Spine:
        case EPackageItemType::DragonBones:
        {
            pii->File = path + pii->File;
            break;
        }

        default:
            break;
        }

        if (ver2)
        {
            FString str = Buffer->ReadS(); //branch
            if (!str.IsEmpty())
                pii->Name = str + "/" + pii->Name;

            int32 branchCnt = Buffer->ReadUbyte();
            if (branchCnt > 0)
            {
                if (branchIncluded)
                {
                    pii->Branches.Emplace();
                    Buffer->ReadSArray(pii->Branches.GetValue(), branchCnt);
                }
                else
                    ItemsByID.Add(Buffer->ReadS(), pii);
            }

            int32 highResCnt = Buffer->ReadUbyte();
            if (highResCnt > 0)
            {
                pii->HighResolution.Emplace();
                Buffer->ReadSArray(pii->HighResolution.GetValue(), highResCnt);
            }
        }

        Items.Push(pii);
        ItemsByID.Add(pii->ID, pii);
        if (!pii->Name.IsEmpty())
            ItemsByName.Add(pii->Name, pii);

        Buffer->SetPos(nextPos);
    }

    Buffer->Seek(indexTablePos, 2);

    cnt = Buffer->ReadShort();
    for (int32 i = 0; i < cnt; i++)
    {
        int32 nextPos = Buffer->ReadShort();
        nextPos += Buffer->GetPos();

        const FString& itemId = Buffer->ReadS();
        const TSharedPtr<FPackageItem>& pii = ItemsByID[Buffer->ReadS()];

        FAtlasSprite* sprite = new FAtlasSprite();
        sprite->Atlas = pii;
        sprite->Rect.Min.X = Buffer->ReadInt();
        sprite->Rect.Min.Y = Buffer->ReadInt();
        sprite->Rect.Max.X = sprite->Rect.Min.X + Buffer->ReadInt();
        sprite->Rect.Max.Y = sprite->Rect.Min.Y + Buffer->ReadInt();
        sprite->bRotated = Buffer->ReadBool();
        if (ver2 && Buffer->ReadBool())
        {
            sprite->Offset.X = Buffer->ReadInt();
            sprite->Offset.Y = Buffer->ReadInt();
            sprite->OriginalSize.X = Buffer->ReadInt();
            sprite->OriginalSize.Y = Buffer->ReadInt();
        }
        else if (sprite->bRotated)
        {
            sprite->Offset.Set(0, 0);
            sprite->OriginalSize.X = sprite->Rect.GetSize().Y;
            sprite->OriginalSize.Y = sprite->Rect.GetSize().X;
        }
        else
        {
            sprite->Offset.Set(0, 0);
            sprite->OriginalSize = sprite->Rect.GetSize();
        }
        Sprites.Add(itemId, sprite);

        Buffer->SetPos(nextPos);
    }

    if (Buffer->Seek(indexTablePos, 3))
    {
        cnt = Buffer->ReadShort();
        for (int32 i = 0; i < cnt; i++)
        {
            int32 nextPos = Buffer->ReadInt();
            nextPos += Buffer->GetPos();

            TSharedPtr<FPackageItem> pii = ItemsByID.FindRef(Buffer->ReadS());
            if (pii.IsValid() && pii->Type == EPackageItemType::Image)
            {
                pii->PixelHitTestData = MakeShareable(new FPixelHitTestData());
                pii->PixelHitTestData->Load(Buffer);
            }

            Buffer->SetPos(nextPos);
        }
    }
}

void* UUIPackage::GetItemAsset(const TSharedPtr<FPackageItem>& Item)
{
    switch (Item->Type)
    {
    case EPackageItemType::Image:
        if (Item->Texture == nullptr)
            LoadImage(Item);
        return Item->Texture;

    case EPackageItemType::Atlas:
        if (Item->Texture == nullptr)
            LoadAtlas(Item);
        return Item->Texture;

    case EPackageItemType::Font:
        if (Item->BitmapFont == nullptr)
            LoadFont(Item);
        return Item->BitmapFont.Get();

    case EPackageItemType::MovieClip:
        if (!Item->MovieClipData.IsValid())
            LoadMovieClip(Item);
        return Item->MovieClipData.Get();

    case EPackageItemType::Sound:
        if (!Item->Sound.IsValid())
            LoadSound(Item);
        return Item->Sound.Get();

    default:
        return nullptr;
    }
}

void UUIPackage::LoadAtlas(const TSharedPtr<FPackageItem>& Item)
{
    UObject* Texture = StaticLoadObject(UTexture2D::StaticClass(), this, *Item->File);
    Item->Texture = NewObject<UNTexture>(this);
    Item->Texture->Init(Cast<UTexture2D>(Texture));
}

void UUIPackage::LoadImage(const TSharedPtr<FPackageItem>& Item)
{
    FAtlasSprite* sprite = Sprites.FindRef(Item->ID);
    if (sprite != nullptr)
    {
        UNTexture* atlas = (UNTexture*)GetItemAsset(sprite->Atlas);
        if (atlas->GetSize() == sprite->Rect.GetSize())
            Item->Texture = atlas;
        else
        {
            Item->Texture = NewObject<UNTexture>(this);
            Item->Texture->Init(atlas, sprite->Rect, sprite->bRotated, sprite->OriginalSize, sprite->Offset);
        }
    }
}

void UUIPackage::LoadMovieClip(const TSharedPtr<FPackageItem>& Item)
{
    TSharedPtr<FMovieClipData> Data = MakeShared<FMovieClipData>();
    Item->MovieClipData = Data;
    FByteBuffer* Buffer = Item->RawData.Get();

    Buffer->Seek(0, 0);

    Data->Interval = Buffer->ReadInt() / 1000.0f;
    Data->bSwing = Buffer->ReadBool();
    Data->RepeatDelay = Buffer->ReadInt() / 1000.0f;

    Buffer->Seek(0, 1);

    int32 frameCount = Buffer->ReadShort();
    Data->Frames.Reserve(frameCount);

    FAtlasSprite* sprite;

    for (int32 i = 0; i < frameCount; i++)
    {
        int32 nextPos = Buffer->ReadShort();
        nextPos += Buffer->GetPos();

        FMovieClipData::Frame Frame;

        FBox2D FrameRect;
        FrameRect.Min.X = Buffer->ReadInt();
        FrameRect.Min.Y = Buffer->ReadInt();
        FrameRect.Max.X = FrameRect.Min.X + Buffer->ReadInt();
        FrameRect.Max.Y = FrameRect.Min.Y + Buffer->ReadInt();
        Frame.AddDelay = Buffer->ReadInt() / 1000.0f;
        const FString& spriteId = Buffer->ReadS();

        if (!spriteId.IsEmpty() && (sprite = Sprites.FindRef(spriteId)) != nullptr)
        {
            Frame.Texture = NewObject<UNTexture>(this);
            Frame.Texture->Init((UNTexture*)GetItemAsset(sprite->Atlas), sprite->Rect, sprite->bRotated, Item->Size, FrameRect.Min);
        }

        Data->Frames.Add(MoveTemp(Frame));

        Buffer->SetPos(nextPos);
    }

    Item->RawData.Reset();
}

void UUIPackage::LoadFont(const TSharedPtr<FPackageItem>& Item)
{
    TSharedPtr<FBitmapFont> BitmapFont = MakeShared<FBitmapFont>();
    Item->BitmapFont = BitmapFont;
    FByteBuffer* Buffer = Item->RawData.Get();

    Buffer->Seek(0, 0);

    bool bTTF = Buffer->ReadBool();
    BitmapFont->bCanTint = Buffer->ReadBool();
    BitmapFont->bResizable = Buffer->ReadBool();
    BitmapFont->bHasChannel = Buffer->ReadBool(); //hasChannel
    BitmapFont->FontSize = Buffer->ReadInt();
    int32 XAdvance = Buffer->ReadInt();
    int32 LineHeight = Buffer->ReadInt();

    FVector2D GlyphTexCoords(0, 0);
    FVector2D GlyphOffset(0, 0);
    FVector2D GlyphSize(0, 0);

    const FAtlasSprite* MainSprite = nullptr;
    if (bTTF && (MainSprite = Sprites.FindRef(Item->ID)) != nullptr)
        BitmapFont->Texture = (UNTexture*)GetItemAsset(MainSprite->Atlas);

    Buffer->Seek(0, 1);

    int32 cnt = Buffer->ReadInt();
    for (int32 i = 0; i < cnt; i++)
    {
        int32 nextPos = Buffer->ReadShort();
        nextPos += Buffer->GetPos();

        FBitmapFont::FGlyph Glyph;

        TCHAR ch = Buffer->ReadUshort();
        const FString& img = Buffer->ReadS();
        GlyphTexCoords.X = Buffer->ReadInt();
        GlyphTexCoords.Y = Buffer->ReadInt();
        GlyphOffset.X = Buffer->ReadInt();
        GlyphOffset.Y = Buffer->ReadInt();
        GlyphSize.X = Buffer->ReadInt();
        GlyphSize.Y = Buffer->ReadInt();
        Glyph.XAdvance = Buffer->ReadInt();
        Glyph.Channel = Buffer->ReadByte();

        if (bTTF)
        {
            FVector2D TexCoords = MainSprite->Rect.Min + GlyphTexCoords;
            Glyph.UVRect = FBox2D(TexCoords / BitmapFont->Texture->GetSize(), (TexCoords + GlyphSize) / BitmapFont->Texture->GetSize());
            Glyph.Offset = GlyphOffset;
            Glyph.Size = GlyphSize;
            Glyph.LineHeight = LineHeight;
        }
        else
        {
            TSharedPtr<FPackageItem> CharImg = GetItem(img);
            if (CharImg.IsValid())
            {
                CharImg = CharImg->GetBranch();
                GlyphSize = CharImg->Size;
                CharImg = CharImg->GetHighResolution();
                GetItemAsset(CharImg);
                Glyph.UVRect = CharImg->Texture->UVRect;

                FVector2D TexScale = GlyphSize / CharImg->Size;

                Glyph.Offset = GlyphOffset + CharImg->Texture->Offset * TexScale;
                Glyph.Size = CharImg->Size * TexScale;

                if (BitmapFont->Texture == nullptr)
                    BitmapFont->Texture = CharImg->Texture->Root;
            }

            if (BitmapFont->FontSize == 0)
                BitmapFont->FontSize = (int32)GlyphSize.Y;

            if (Glyph.XAdvance == 0)
            {
                if (XAdvance == 0)
                    Glyph.XAdvance = GlyphOffset.X + GlyphSize.X;
                else
                    Glyph.XAdvance = XAdvance;
            }

            Glyph.LineHeight = GlyphOffset.Y < 0 ? GlyphSize.Y : (GlyphOffset.Y + GlyphSize.Y);
            if (Glyph.LineHeight < BitmapFont->FontSize)
                Glyph.LineHeight = BitmapFont->FontSize;
        }

        BitmapFont->Glyphs.Add(ch, Glyph);
        Buffer->SetPos(nextPos);
    }

    Item->RawData.Reset();
}

void UUIPackage::LoadSound(const TSharedPtr<FPackageItem>& Item)
{
    TSharedPtr<FSlateSound> Sound = MakeShared<FSlateSound>();
    Item->Sound = Sound;

    UObject* SoundObject = StaticLoadObject(USoundBase::StaticClass(), this, *Item->File);
    Sound->SetResourceObject(SoundObject);
}

UUIPackageStatic* UUIPackageStatic::Singleton = nullptr;

UUIPackageStatic& UUIPackageStatic::Get()
{
    if (Singleton == nullptr)
    {
        Singleton = NewObject<UUIPackageStatic>();
        Singleton->AddToRoot();
    }
    return *Singleton;
}

void UUIPackageStatic::Destroy()
{
    if (Singleton != nullptr)
    {
        Singleton->RemoveFromRoot();
        Singleton = nullptr;
    }
}
