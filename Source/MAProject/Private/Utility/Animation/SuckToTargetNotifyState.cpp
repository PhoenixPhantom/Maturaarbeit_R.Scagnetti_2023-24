// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/Animation/SuckToTargetNotifyState.h"

#include "Characters/GeneralCharacter.h"
#include "Utility/Animation/SuckToTargetComponent.h"

USuckToTargetNotifyState::USuckToTargetNotifyState()
{
	bShouldFireInEditor = false;
	NotifyColor = FColor(0, 255, 0);
}

void USuckToTargetNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                  float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	UActorComponent* Component = MeshComp->GetOwner()->GetComponentByClass(USuckToTargetComponent::StaticClass());
	if(!IsValid(Component)) return;
	CastChecked<USuckToTargetComponent>(Component)->StartWarping(TotalDuration, FStartMotionWarpingKey());
}
