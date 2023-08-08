// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/Animation/AnimNotifyState_MeleeAttack.h"

#include "Characters/Fighters/FighterCharacter.h"

UAnimNotifyState_MeleeAttack::UAnimNotifyState_MeleeAttack() : bStartEmpty(true), bAllowHitRecentVictims(true), bRefreshHitActors(true)
{
	bShouldFireInEditor = false;
	NotifyColor = {255, 0, 0};
}

void UAnimNotifyState_MeleeAttack::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                          float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	MeshOwner = CastChecked<AFighterCharacter>(MeshComp->GetOwner());
	MeshOwner->ActivateMeleeBones(BonesToEnable, bStartEmpty, bAllowHitRecentVictims, FMeleeControlsKey());
	
}   

void UAnimNotifyState_MeleeAttack::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if(!IsValid(MeshOwner)) return; //it is possible that the MeshOwner was killed since assignment
	MeshOwner->DeactivateMeleeBones(BonesToEnable, bRefreshHitActors, FMeleeControlsKey());
	MeshOwner = nullptr;
}

