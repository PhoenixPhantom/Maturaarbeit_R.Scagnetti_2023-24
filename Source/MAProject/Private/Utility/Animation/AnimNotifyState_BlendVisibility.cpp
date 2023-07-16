// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/Animation/AnimNotifyState_BlendVisibility.h"

#include "AnimationEditorPreviewActor.h"

FMeshInterpolationUnit::FMeshInterpolationUnit(): DynamicMaterialInstance(nullptr), StartingVisibility(0.f), IncreaseRate(0.f)
{}

FMeshInterpolationUnit::FMeshInterpolationUnit(UMaterialInstanceDynamic* NewInstance, float NewStartingVisibility,
                                             float NewIncreaseRate): DynamicMaterialInstance(NewInstance),
                                                                     StartingVisibility(NewStartingVisibility),IncreaseRate(NewIncreaseRate)
{}

UAnimNotifyState_BlendVisibility::UAnimNotifyState_BlendVisibility() : PassedTime(0.f), FinalVisibility(0.5f)
{
	bShouldFireInEditor = true;
	NotifyColor = FColor(255, 0, 255);
}

void UAnimNotifyState_BlendVisibility::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                              float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	PassedTime = 0.f;
	InterpolationUnits.Empty();
	for(int32 i = 0; i < MeshComp->GetNumMaterials(); i++)
	{
		UMaterialInstanceDynamic* DynamicInstance = MeshComp->CreateDynamicMaterialInstance(i);
		if(!IsValid(DynamicInstance) ||
			InterpolationUnits.FindByPredicate([DynamicInstance](const FMeshInterpolationUnit& InterpolationUnit)
			{
				return InterpolationUnit.DynamicMaterialInstance == DynamicInstance;
			}) != nullptr) continue;
		float CurrentVisibility;
		DynamicInstance->GetScalarParameterValue(VisibilitySettingName, CurrentVisibility);
		InterpolationUnits.Add(FMeshInterpolationUnit(DynamicInstance, CurrentVisibility
			, (FinalVisibility - CurrentVisibility) / TotalDuration));
	}
}

void UAnimNotifyState_BlendVisibility::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	PassedTime += FrameDeltaTime;
	for(const FMeshInterpolationUnit& MeshInterpolationUnit : InterpolationUnits)
	{
		const float Value = MeshInterpolationUnit.StartingVisibility + PassedTime * MeshInterpolationUnit.IncreaseRate;
		MeshInterpolationUnit.DynamicMaterialInstance->SetScalarParameterValue(VisibilitySettingName,
			MeshInterpolationUnit.IncreaseRate >= 0.f ?
			FMath::Clamp(Value, 0.f, FinalVisibility) : FMath::Clamp(Value, FinalVisibility, 1.f));
	}
}


UAnimNotify_InEditorResetVisibility::UAnimNotify_InEditorResetVisibility()
{
	bShouldFireInEditor = true;
	NotifyColor = FColor(50, 50, 50, 100);
}

void UAnimNotify_InEditorResetVisibility::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                              const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if(!bShouldFireInEditor ||
		!MeshComp->GetOuter()->GetClass()->IsChildOf(AAnimationEditorPreviewActor::StaticClass())) return;
	
	for(int32 i = 0; i < MeshComp->GetNumMaterials(); i++)
	{
		UMaterialInstanceDynamic* DynamicInstance = MeshComp->CreateDynamicMaterialInstance(i);
		if(!IsValid(DynamicInstance)) return;
		DynamicInstance->SetScalarParameterValue(VisibilitySettingName, 1.f);
	}
}
