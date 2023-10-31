// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_BlendVisibility.generated.h"

class AGeneralCharacter;

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

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
	
protected:
	float PassedTime;
	float StartingVisibility;
	float IncreaseRate;

	UPROPERTY()
	AGeneralCharacter* OwningCharacter;

	UPROPERTY(EditAnywhere)
	uint8 bBlockOtherVisibilityChanges:1;
	UPROPERTY(EditAnywhere)
	uint8 bEndBlocking:1;
	
	UPROPERTY(EditAnywhere, meta=(ClampMin = 0.0, UIMin=0.0, ClampMax=1.0, UIMax=1.0))
	float FinalVisibility;
};

UCLASS()
class MAPROJECT_API UAnimNotifyState_ForceConstantVisibility : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	UAnimNotifyState_ForceConstantVisibility();
	
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
							 const FAnimNotifyEventReference& EventReference) override;
	
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime,
		const FAnimNotifyEventReference& EventReference) override;
	
protected:
	UPROPERTY()
	AGeneralCharacter* OwningCharacter;
	
	UPROPERTY(EditAnywhere, meta=(ClampMin = 0.0, UIMin=0.0, ClampMax=1.0, UIMax=1.0))
	float FinalVisibility;
};


UCLASS(meta=(ToolTip="Used to set the visibility to 1 in the animation editor (use UAnimNotifyState_BlendVisibility instead to do this In-Game)"))
class MAPROJECT_API UAnimNotify_InEditorResetVisibility : public UAnimNotify
{
	GENERATED_BODY()
public:
	UAnimNotify_InEditorResetVisibility();
	virtual bool IsEditorOnly() const override{ return true; };
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
