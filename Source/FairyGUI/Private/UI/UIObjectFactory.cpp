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
FGLoaderCreator FUIObjectFactory::LoaderCreator;

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
    SetExtension(URL, FGComponentCreator::CreateLambda([ClassType]() {
        return ::NewObject<UGComponent>(GetTransientPackage(), ClassType);
    }));
}

UGObject* FUIObjectFactory::NewObject(const TSharedPtr<FPackageItem>& PackageItem)
{
    UGObject* obj = nullptr;
    if (PackageItem->ExtensionCreator.IsBound())
        obj = PackageItem->ExtensionCreator.Execute();
    else
        obj = NewObject(PackageItem->ObjectType);
    if (obj != nullptr)
        obj->PackageItem = PackageItem;

    return obj;
}

UGObject* FUIObjectFactory::NewObject(EObjectType Type)
{
    switch (Type)
    {
    case EObjectType::Image:
        return ::NewObject<UGImage>();

    case EObjectType::MovieClip:
        return ::NewObject<UGMovieClip>();

    case EObjectType::Component:
        return ::NewObject<UGComponent>();

    case EObjectType::Text:
        return ::NewObject<UGTextField>();

    case EObjectType::RichText:
        return ::NewObject<UGRichTextField>();

    case EObjectType::InputText:
        return ::NewObject<UGTextInput>();

    case EObjectType::Group:
        return ::NewObject<UGGroup>();

    case EObjectType::List:
        return ::NewObject<UGList>();

    case EObjectType::Graph:
        return ::NewObject<UGGraph>();

    case EObjectType::Loader:
        if (LoaderCreator.IsBound())
            return LoaderCreator.Execute();
        else
            return ::NewObject<UGLoader>();

    case EObjectType::Button:
        return ::NewObject<UGButton>();

    case EObjectType::Label:
        return ::NewObject<UGLabel>();

    case EObjectType::ProgressBar:
        return ::NewObject<UGProgressBar>();

    case EObjectType::Slider:
        return ::NewObject<UGSlider>();

    case EObjectType::ScrollBar:
        return ::NewObject<UGScrollBar>();

    case EObjectType::ComboBox:
        return ::NewObject<UGComboBox>();

    case EObjectType::Tree:
        return ::NewObject<UGTree>();

    case EObjectType::Loader3D:
        return ::NewObject<UGLoader3D>();

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
