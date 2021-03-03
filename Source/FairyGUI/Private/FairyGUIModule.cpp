// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "FairyGUIModule.h"
#if WITH_EDITOR
#include "Editor.h"
#include "FairyApplication.h"
#endif
#include "UI/UIConfig.h"

#define LOCTEXT_NAMESPACE "FFairyGUIModule"

void FFairyGUIModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

#if WITH_EDITOR
    EndPieDelegateHandle = FEditorDelegates::EndPIE.AddLambda([](bool boolSent) {
        UE_LOG(LogFairyGUI, Log, TEXT("Application destroy"));

        UFairyApplication::Destroy();
    });
#endif
}

void FFairyGUIModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

#if WITH_EDITOR
    FEditorDelegates::EndPIE.Remove(EndPieDelegateHandle);
#endif
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FFairyGUIModule, FairyGUI)