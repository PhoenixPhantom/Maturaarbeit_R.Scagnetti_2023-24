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
	
	UPROPERTY(EditAnywhere, Category = Combat, AdvancedDisplay,
		meta=(ToolTip="The time after finishing the animation that it takes until this Attack can be used again (negative values are permitted)"))
	float CdTime;
	UPROPERTY(EditAnywhere, Category = Combat)
	FAttackDamageEvent DamageEvent;
	UPROPERTY(EditAnywhere, Category = Animation)
	UAnimMontage* AtkAnimation;
	UPROPERTY(EditAnywhere, Category = Animation, AdvancedDisplay)
	FInputLimits ResultingLimits;


	bool operator==(const FAttackProperties& AttackProperties) const;


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
