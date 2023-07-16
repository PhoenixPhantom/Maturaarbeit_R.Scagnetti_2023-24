// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimNotifyState_TimedNiagaraEffect.h"
#include "AnimNotifyState_TimedNiagaraEffectParameters.generated.h"

USTRUCT()
struct FObjectParameter
{
	GENERATED_BODY()
	FObjectParameter(): Object(nullptr){}

	UPROPERTY(EditAnywhere)
	FString VariableName;

	UPROPERTY(EditAnywhere)
	UObject* Object;
};

/**
 * 
 */
UCLASS()
class MAPROJECT_API UAnimNotifyState_TimedNiagaraEffectParameters : public UAnimNotifyState_TimedNiagaraEffect
{
	GENERATED_BODY()
public:
	UAnimNotifyState_TimedNiagaraEffectParameters();
	
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere)
	TArray<FObjectParameter> ObjectParameters;
};
