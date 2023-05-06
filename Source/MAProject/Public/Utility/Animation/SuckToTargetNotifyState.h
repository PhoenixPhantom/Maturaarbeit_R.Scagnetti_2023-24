// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "SuckToTargetNotifyState.generated.h"

class AGeneralCharacter;

/**
 * 
 */
UCLASS()
class MAPROJECT_API USuckToTargetNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	USuckToTargetNotifyState();
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
	                         const FAnimNotifyEventReference& EventReference) override;
};
