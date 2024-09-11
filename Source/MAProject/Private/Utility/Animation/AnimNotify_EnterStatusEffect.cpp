// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/Animation/AnimNotify_EnterStatusEffect.h"

#include "Characters/GeneralCharacter.h"

UAnimNotify_EnterStatusEffect::UAnimNotify_EnterStatusEffect()
{
	bShouldFireInEditor = false;
	NotifyColor = FColor(212, 175, 55);
}

void UAnimNotify_EnterStatusEffect::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                           const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	CastChecked<AGeneralCharacter>(MeshComp->GetOwner())->ReceiveStatusEffectExternal(StatusEffectType, FModifyCharacterStatusEffectKey());
}
