// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/Animation/AnimNotifyState_ForceNoLegIK.h"

#include "Characters/GeneralCharacter.h"
#include "Utility/Animation/CustomAnimInstance.h"

UAnimNotifyState_ForceNoLegIK::UAnimNotifyState_ForceNoLegIK(): BlockedIKTypes(~0)
{
	bShouldFireInEditor = false;
}

void UAnimNotifyState_ForceNoLegIK::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	const AGeneralCharacter* MeshOwner = CastChecked<AGeneralCharacter>(MeshComp->GetOwner());
	MeshOwner->GetCustomAnimInstance()->SetAllowedLegIKTypes(~BlockedIKTypes, TotalDuration);
}
 