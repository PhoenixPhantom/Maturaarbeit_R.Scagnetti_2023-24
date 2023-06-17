// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttackDamageEvent.h"
#include "Characters/InputManagement.h"
#include "AttackProperties.generated.h"

USTRUCT()
struct MAPROJECT_API FAttackProperties
{
	GENERATED_BODY()
	
	FAttackProperties();

	FAttackProperties(const FAttackProperties& Properties);

	
	UPROPERTY(EditAnywhere, Category = Combat, meta=(Units="%"))
	float DamagePercent;
	
	UPROPERTY(EditAnywhere, Category = Combat, AdvancedDisplay, meta=(ForceUnits="s",
		ToolTip="The time after finishing the animation that it takes until this Attack can be used again (negative values are permitted)"))
	float CdTime;
	UPROPERTY(EditAnywhere, Category = Combat)
	FAttackDamageEvent DamageEvent;
	UPROPERTY(EditAnywhere, Category = Combat,
		meta=(Units="cm", ToolTip="The maximal distance that motion warping can take the animation. If the target is further away, the character still moves in direction of the target but doesn't go the whole way."))
	float MaximalMovementDistance;
	UPROPERTY(EditAnywhere, Category = Combat,
		meta=(Units="cm", ToolTip="Default distance that the animation will move the actor."))
	float DefaultMovementDistance;
	UPROPERTY(EditAnywhere, Category = Opponent, AdvancedDisplay)
	float Priority;
	UPROPERTY(EditAnywhere, Category = Animation)
	UAnimMontage* AtkAnimation;
	UPROPERTY(EditAnywhere, Category = Animation, AdvancedDisplay, meta=(ToolTip="The limits that will be applied one after another in order of indices on attack execution."))
	TArray<FInputLimits> InputLimits;


	bool operator==(const FAttackProperties& AttackProperties) const;

	float GetPriority(float DistanceFromTarget) const;
	bool GetIsOnCd() const { return bIsOnCd; }
	void Execute();
	float CdTimeElapsed() const;
	float CdTimeRemaining() const;
	float GetTotalCdTime() const;

	UPROPERTY()
	UWorld* World;
	
protected:
	bool bIsOnCd;
	FTimerHandle CdHandle;
};
