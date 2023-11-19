// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/Animation/AnimNotifyState_MeleeAttack.h"

#include "Characters/Fighters/FighterCharacter.h"

UAnimNotifyState_MeleeAttack::UAnimNotifyState_MeleeAttack() : MeshOwner(nullptr), bStartEmpty(true),
	bAllowHitRecentVictims(true), bIsLastAttack(true)
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
	MeshComp->GetWorld()->GetTimerManager().SetTimer(EndTimerHandle, this,
		&UAnimNotifyState_MeleeAttack::EndAttack, TotalDuration);	
}

void UAnimNotifyState_MeleeAttack::EndAttack()
{
	if(!IsValid(MeshOwner)) return; //it is possible that the MeshOwner was killed since assignment
	MeshOwner->DeactivateMeleeBones(BonesToEnable, bIsLastAttack, FMeleeControlsKey());
	MeshOwner = nullptr;
}

