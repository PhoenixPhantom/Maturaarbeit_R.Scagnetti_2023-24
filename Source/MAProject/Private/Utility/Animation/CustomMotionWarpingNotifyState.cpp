// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/Animation/CustomMotionWarpingNotifyState.h"

#include "Characters/GeneralCharacter.h"

UCustomMotionWarpingNotifyState::UCustomMotionWarpingNotifyState(): MeshOwner(nullptr)
{
	bShouldFireInEditor = false;
	NotifyColor = FColor(0, 255, 0);
}

void UCustomMotionWarpingNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                  float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	MeshOwner = CastChecked<AGeneralCharacter>(MeshComp->GetOwner());
	MeshOwner->StartMotionWarping(TotalDuration, FMotionWarpingKey());
}
