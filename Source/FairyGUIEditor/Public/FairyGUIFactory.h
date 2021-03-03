#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "EditorReimportHandler.h"
#include "FairyGUIFactory.generated.h"

UCLASS()
class FAIRYGUIEDITOR_API UFairyGUIFactory : public UFactory, public  FReimportHandler
{
	GENERATED_BODY()

public:
	UFairyGUIFactory();
	virtual UObject* FactoryCreateBinary(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn) override;
	virtual bool CanReimport(UObject* Obj, TArray<FString>& OutFilenames)  override;
	virtual void SetReimportPaths(UObject* Obj, const TArray<FString>&  NewReimportPaths) override;
	virtual EReimportResult::Type Reimport(UObject* Obj) override;
};
