// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_BlendVisibility.generated.h"

struct FMeshInterpolationUnit
{
	FMeshInterpolationUnit();
	FMeshInterpolationUnit(UMaterialInstanceDynamic* NewInstance,
	                      float NewStartingVisibility, float NewIncreaseRate);

	UMaterialInstanceDynamic* DynamicMaterialInstance;
	float StartingVisibility;
	float IncreaseRate;
};

/**
 * 
 */
UCLASS()
class MAPROJECT_API UAnimNotifyState_BlendVisibility : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	UAnimNotifyState_BlendVisibility();
	
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
							 const FAnimNotifyEventReference& EventReference) override;
	
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime,
		const FAnimNotifyEventReference& EventReference) override;
	
protected:
	float PassedTime;
	TArray<FMeshInterpolationUnit> InterpolationUnits;

	UPROPERTY(EditAnywhere)
	FName VisibilitySettingName;
	
	UPROPERTY(EditAnywhere, meta=(ClampMin = 0.0, UIMin=0.0, ClampMax=1.0, UIMax=1.0))
	float FinalVisibility;
};


UCLASS(meta=(ToolTip="Used to set the visibility to 1 in the animation editor (use UAnimNotifyState_BlendVisibility instead to do this In-Game)"))
class MAPROJECT_API UAnimNotify_InEditorResetVisibility : public UAnimNotify
{
	GENERATED_BODY()
public:
	UAnimNotify_InEditorResetVisibility();
	virtual bool IsEditorOnly() const override{ return false; };
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
	
protected:
	UPROPERTY(EditAnywhere)
	FName VisibilitySettingName;
};
