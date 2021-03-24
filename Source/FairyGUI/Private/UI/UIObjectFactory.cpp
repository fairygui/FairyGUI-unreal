#include "UI/UIObjectFactory.h"
#include "UI/UIPackage.h"
#include "UI/PackageItem.h"
#include "UI/GComponent.h"
#include "UI/GImage.h"
#include "UI/GMovieClip.h"
#include "UI/GTextField.h"
#include "UI/GRichTextField.h"
#include "UI/GTextInput.h"
#include "UI/GGraph.h"
#include "UI/GLoader.h"
#include "UI/GLoader3D.h"
#include "UI/GGroup.h"
#include "UI/GLabel.h"
#include "UI/GButton.h"
#include "UI/GComboBox.h"
#include "UI/GProgressBar.h"
#include "UI/GSlider.h"
#include "UI/GScrollBar.h"
#include "UI/GList.h"
#include "UI/GTree.h"

TMap<FString, FGComponentCreator> FUIObjectFactory::PackageItemExtensions;
TSubclassOf<UGLoader> FUIObjectFactory::LoaderExtension;

void FUIObjectFactory::SetExtension(const FString& URL, FGComponentCreator Creator)
{
    if (URL.IsEmpty())
    {
        UE_LOG(LogFairyGUI, Warning, TEXT("Invaild url: %s"), *URL);
        return;
    }

    TSharedPtr<FPackageItem> PackageItem = UUIPackage::GetItemByURL(URL);
    if (PackageItem.IsValid())
        PackageItem->ExtensionCreator = Creator;

    PackageItemExtensions.Add(URL, Creator);
}

void FUIObjectFactory::SetExtension(const FString& URL, TSubclassOf<UGComponent> ClassType)
{
    SetExtension(URL, FGComponentCreator::CreateLambda([ClassType](UObject* Outer) {
        return ::NewObject<UGComponent>(Outer, ClassType);
    }));
}

UGObject* FUIObjectFactory::NewObject(const TSharedPtr<FPackageItem>& PackageItem, UObject* Outer)
{
    UGObject* obj = nullptr;
    if (PackageItem->ExtensionCreator.IsBound())
        obj = PackageItem->ExtensionCreator.Execute(Outer);
    else
        obj = NewObject(PackageItem->ObjectType, Outer);
    if (obj != nullptr)
        obj->PackageItem = PackageItem;

    return obj;
}

UGObject* FUIObjectFactory::NewObject(EObjectType Type, UObject* Outer)
{
    switch (Type)
    {
    case EObjectType::Image:
        return ::NewObject<UGImage>(Outer);

    case EObjectType::MovieClip:
        return ::NewObject<UGMovieClip>(Outer);

    case EObjectType::Component:
        return ::NewObject<UGComponent>(Outer);

    case EObjectType::Text:
        return ::NewObject<UGTextField>(Outer);

    case EObjectType::RichText:
        return ::NewObject<UGRichTextField>(Outer);

    case EObjectType::InputText:
        return ::NewObject<UGTextInput>(Outer);

    case EObjectType::Group:
        return ::NewObject<UGGroup>(Outer);

    case EObjectType::List:
        return ::NewObject<UGList>(Outer);

    case EObjectType::Graph:
        return ::NewObject<UGGraph>(Outer);

    case EObjectType::Loader:
        if (LoaderExtension != nullptr)
            return ::NewObject<UGLoader>(Outer, LoaderExtension);
        else
            return ::NewObject<UGLoader>(Outer);

    case EObjectType::Button:
        return ::NewObject<UGButton>(Outer);

    case EObjectType::Label:
        return ::NewObject<UGLabel>(Outer);

    case EObjectType::ProgressBar:
        return ::NewObject<UGProgressBar>(Outer);

    case EObjectType::Slider:
        return ::NewObject<UGSlider>(Outer);

    case EObjectType::ScrollBar:
        return ::NewObject<UGScrollBar>(Outer);

    case EObjectType::ComboBox:
        return ::NewObject<UGComboBox>(Outer);

    case EObjectType::Tree:
        return ::NewObject<UGTree>(Outer);

    case EObjectType::Loader3D:
        return ::NewObject<UGLoader3D>(Outer);

    default:
        return nullptr;
    }
}

void FUIObjectFactory::ResolvePackageItemExtension(const TSharedPtr<FPackageItem>& PackageItem)
{
    auto it = PackageItemExtensions.Find("ui://" + PackageItem->Owner->GetID() + PackageItem->ID);
    if (it != nullptr)
    {
        PackageItem->ExtensionCreator = *it;
        return;
    }
    it = PackageItemExtensions.Find("ui://" + PackageItem->Owner->GetName() + "/" + PackageItem->Name);
    if (it != nullptr)
    {
        PackageItem->ExtensionCreator = *it;
        return;
    }
    PackageItem->ExtensionCreator = nullptr;
}
