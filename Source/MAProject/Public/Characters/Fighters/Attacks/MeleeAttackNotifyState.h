// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "MeleeAttackNotifyState.generated.h"

class AFighterCharacter;

/**
 * 
 */
UCLASS()
class MAPROJECT_API UMeleeAttackNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	UMeleeAttackNotifyState();
	
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
							 const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
protected:
	UPROPERTY()
	AFighterCharacter* MeshOwner;
	
	UPROPERTY(EditAnywhere)
	TArray<FName> BonesToEnable;

	UPROPERTY(EditAnywhere, meta=(ToolTip="Wether we should delete all prior enabled bones (to ensure there aren't to many enabled). It is reccomended to set the first MeleeAttackNotifyState in each animation to true."))
	bool bStartEmpty;

	UPROPERTY(EditAnywhere, AdvancedDisplay, meta=(ToolTip="Wether this attack string has ended and once hit actors can be damaged again"))
	bool bRefreshHitActors;
};
