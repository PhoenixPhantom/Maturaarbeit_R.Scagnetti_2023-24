// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimNotifyState_TimedNiagaraEffect.h"
#include "AnimNotifyState_TimedNiagaraEffectParameters.generated.h"


USTRUCT()
struct FMaterialParameter
{
	GENERATED_BODY()
	FMaterialParameter(): Material(nullptr)	{}

	UPROPERTY(EditAnywhere)
	FName VariableName;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* Material;
};

USTRUCT()
struct FFloatParameter
{
	GENERATED_BODY()
	FFloatParameter(): Value(0.f), bUseMeshScaling(false){}

	UPROPERTY(EditAnywhere)
	FName VariableName;

	UPROPERTY(EditAnywhere)
	float Value;

	UPROPERTY(EditAnywhere)
	bool bUseMeshScaling;
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
	TArray<FMaterialParameter> MaterialParameters;

	UPROPERTY(EditAnywhere, meta=(ToolTip="Name of the Niagara variable that stores the desired playback length (leave empty if none exists)"))
	FName PlayLengthName;

	virtual UFXSystemComponent* SpawnEffect(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) const override;
};
