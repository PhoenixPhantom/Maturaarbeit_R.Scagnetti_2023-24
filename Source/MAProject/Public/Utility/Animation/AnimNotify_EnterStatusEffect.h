// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_EnterStatusEffect.generated.h"

class UStatusEffect;
/**
 * 
 */
UCLASS()
class MAPROJECT_API UAnimNotify_EnterStatusEffect : public UAnimNotify
{
	GENERATED_BODY()
public:
	UAnimNotify_EnterStatusEffect();	
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UStatusEffect> StatusEffectType;
};
