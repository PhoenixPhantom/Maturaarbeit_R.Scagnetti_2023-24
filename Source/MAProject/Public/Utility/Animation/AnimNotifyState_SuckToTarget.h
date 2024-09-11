// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_SuckToTarget.generated.h"

class AGeneralCharacter;

/**
 * 
 */
UCLASS()
class MAPROJECT_API UAnimNotifyState_SuckToTarget : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	UAnimNotifyState_SuckToTarget();
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
	                         const FAnimNotifyEventReference& EventReference) override;
};
