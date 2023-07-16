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

USTRUCT()
struct FFloatParameter
{
	GENERATED_BODY()
	FFloatParameter(): Value(0.f){}

	UPROPERTY(EditAnywhere)
	FString VariableName;

	UPROPERTY(EditAnywhere)
	float Value;
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

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
		const FAnimNotifyEventReference& EventReference) override;
	
protected:
	float NotifyTime;
	
	UPROPERTY(EditAnywhere)
	TArray<FFloatParameter> FloatParameters;
	
	UPROPERTY(EditAnywhere)
	TArray<FObjectParameter> ObjectParameters;

	UPROPERTY(EditAnywhere, meta=(ToolTip="Name of the Niagara variable that stores the desired playback length (leave empty if none exists)"))
	FString PlayLengthName;

	virtual UFXSystemComponent* SpawnEffect(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) const override;
};
