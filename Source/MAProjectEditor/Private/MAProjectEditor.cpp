#include "MAProjectEditor.h"

#define LOCTEXT_NAMESPACE "FMAProjectEditorModule"

DEFINE_LOG_CATEGORY(MAProjectEditor)

void FMAProjectEditorModule::StartupModule()
{
	UE_LOG(MAProjectEditor, Warning, TEXT("MAProjectEditor: Log Started"));
}

void FMAProjectEditorModule::ShutdownModule()
{
	UE_LOG(MAProjectEditor, Warning, TEXT("MAProjectEditor: Log Ended"));
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FMAProjectEditorModule, MAProjectEditor)