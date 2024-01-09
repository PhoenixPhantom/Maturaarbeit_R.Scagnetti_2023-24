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

	
	UPROPERTY(EditAnywhere, Category = Combat, meta=(Units="%", UIMin=0, ClampMin=0))
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
	UPROPERTY(EditAnywhere, Category = Combat,
		meta=(Units="cm", ToolTip="The character's animation cannot move less than the minimal movement distance (only has an effect on opponents)"))
	float MinimalMovementDistance;
	UPROPERTY(EditAnywhere, Category = Combat, AdvancedDisplay, meta=(ForceUnits="s",
		ToolTip="The maximum time the character can wait after this attack to continue the combo (reccomendation: around 2.5s for players and around 10s for opponents)"))
	float MaxComboTime;
	UPROPERTY(EditAnywhere, Category = Combat, AdvancedDisplay, meta=(UIMin=0, ClampMin=0,
		ToolTip="Attack priority (compared to other attacks) only has an effect on opponents"))
	float Priority;
	UPROPERTY(EditAnywhere, Category = Combat, AdvancedDisplay,
		meta=(ToolTip="Whether the attack needs to connect (only has an effect on opponents)"))
	bool bIsMeleeAttack;
	UPROPERTY(EditAnywhere, Category = Animation)
	UAnimMontage* AtkAnimation;
	UPROPERTY(EditAnywhere, Category = Animation, meta=(ToolTip="The limits that will be applied one after another in order of indices on attack execution."))
	TArray<FNewInputLimits> InputLimits;


	bool operator==(const FAttackProperties& AttackProperties) const;
	
	float GetOverallValue() const;
	float GetTotalCdTime() const;
};
