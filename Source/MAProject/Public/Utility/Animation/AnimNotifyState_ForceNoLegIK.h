﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_ForceNoLegIK.generated.h"

/**
 * 
 */
UCLASS()
class MAPROJECT_API UAnimNotifyState_ForceNoLegIK : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	UAnimNotifyState_ForceNoLegIK();

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
							 const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere, meta=(Bitmask, BitmaskEnum = "/Script/MAProject.ELegIKType"))
	int32 BlockedIKTypes;	
};
