// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/Animation/AnimNotifyState_BlendVisibility.h"

#include "AnimationEditorPreviewActor.h"
#include "Characters/GeneralCharacter.h"

UAnimNotifyState_BlendVisibility::UAnimNotifyState_BlendVisibility() : PassedTime(0.f), StartingVisibility(0.f),
	IncreaseRate(0.f), OwningCharacter(nullptr), bBlockOtherVisibilityChanges(true),
	bEndBlocking(false), FinalVisibility(0.5f)
{
	bShouldFireInEditor = true;
	NotifyColor = FColor(255, 0, 255);
}

void UAnimNotifyState_BlendVisibility::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                   float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	PassedTime = 0.f;
	
	OwningCharacter = Cast<AGeneralCharacter>(MeshComp->GetOwner());
	if(IsValid(OwningCharacter))
	{
		StartingVisibility = OwningCharacter->GetMeshesOpacity();
		OwningCharacter->SetAllowAutomaticOpacityChanges(bBlockOtherVisibilityChanges, FSetCharacterOpacity());
	}
	else
	{
		StartingVisibility = MeshComp->GetCustomPrimitiveData().Data.IsEmpty() ? 0.f :
			MeshComp->GetCustomPrimitiveData().Data[0];
	}
	IncreaseRate = (FinalVisibility - StartingVisibility) / TotalDuration;
}

void UAnimNotifyState_BlendVisibility::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	PassedTime += FrameDeltaTime;
	const float TheoreticalValue = StartingVisibility + PassedTime * IncreaseRate;
	const float NewVisibility = IncreaseRate >= 0.f ? FMath::Clamp(TheoreticalValue, 0.f, FinalVisibility) :
		FMath::Clamp(TheoreticalValue, FinalVisibility, 1.f);
	if(IsValid(OwningCharacter))
	{
		OwningCharacter->SetMeshesOpacity(NewVisibility, FSetCharacterOpacity());
	}
	else
	{
		MeshComp->SetCustomPrimitiveDataFloat(0, NewVisibility);
	}
}

void UAnimNotifyState_BlendVisibility::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if(bEndBlocking && IsValid(OwningCharacter))
	{
		OwningCharacter->SetAllowAutomaticOpacityChanges(true, FSetCharacterOpacity());
	}
}


UAnimNotifyState_ForceConstantVisibility::UAnimNotifyState_ForceConstantVisibility() : OwningCharacter(nullptr),
	FinalVisibility(0.f)
{
}

void UAnimNotifyState_ForceConstantVisibility::NotifyBegin(USkeletalMeshComponent* MeshComp,
                                                           UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	OwningCharacter = Cast<AGeneralCharacter>(MeshComp->GetOwner());
	if(IsValid(OwningCharacter))
	{
		OwningCharacter->SetAllowAutomaticOpacityChanges(true, FSetCharacterOpacity());
	}
}

void UAnimNotifyState_ForceConstantVisibility::NotifyTick(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	if(IsValid(OwningCharacter))
	{
		OwningCharacter->SetMeshesOpacity(FinalVisibility, FSetCharacterOpacity());
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
	
	AGeneralCharacter* OwningCharacter = Cast<AGeneralCharacter>(MeshComp->GetOwner());
	if(IsValid(OwningCharacter))
	{
		OwningCharacter->SetMeshesOpacity(0.f, FSetCharacterOpacity());
	}
	else
	{
		MeshComp->SetCustomPrimitiveDataFloat(0, 0.f);
	}
}
