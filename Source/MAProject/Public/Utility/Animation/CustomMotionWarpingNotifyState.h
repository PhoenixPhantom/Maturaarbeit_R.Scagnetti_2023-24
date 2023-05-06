// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "CustomMotionWarpingNotifyState.generated.h"

class AGeneralCharacter;

/**
 * 
 */
UCLASS()
class MAPROJECT_API UCustomMotionWarpingNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	UCustomMotionWarpingNotifyState();
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
	                         const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY()
	AGeneralCharacter* MeshOwner;
};
