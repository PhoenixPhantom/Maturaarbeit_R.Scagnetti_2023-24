// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_MeleeAttack.generated.h"

class AFighterCharacter;

/**
 * 
 */
UCLASS()
class MAPROJECT_API UAnimNotifyState_MeleeAttack : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	UAnimNotifyState_MeleeAttack();
	
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
							 const FAnimNotifyEventReference& EventReference) override;

protected:
	FTimerHandle EndTimerHandle;
	UPROPERTY()
	AFighterCharacter* MeshOwner;
	
	UPROPERTY(EditAnywhere)
	TArray<FName> BonesToEnable;

	UPROPERTY(EditAnywhere, AdvancedDisplay, meta=(ToolTip="Wether we should delete all prior enabled bones (to ensure there aren't to many enabled). It is reccomended to set the first MeleeAttackNotifyState in each animation to true."))
	bool bStartEmpty;

	UPROPERTY(EditAnywhere, AdvancedDisplay, meta=(ToolTip="Wether this attack can damage targets even if they were already damaged from the last attack of the string"))
	bool bAllowHitRecentVictims;

	UPROPERTY(EditAnywhere, AdvancedDisplay, meta=(ToolTip="Wether this was the last attack in this animation"))
	bool bIsLastAttack;

	UFUNCTION()
	void EndAttack();
};
