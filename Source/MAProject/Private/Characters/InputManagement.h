// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InputManagement.generated.h"

UENUM()
enum class EInputType
{
	Walk,
	Sprint,
	Camera,
	Jump,
	Attack,
	SwitchOut,
	//Force is the strongest input type and cannot be blocked or stopped from execution
	Force
	
};

USTRUCT()
struct FInputLimits
{
	GENERATED_BODY()

	FInputLimits();
	FInputLimits(const FInputLimits& Limits) = default;

	bool operator==(const FInputLimits& Compare) const;

	///@brief The type of input that these limits are applied by
	UPROPERTY(EditAnywhere)
	EInputType LimiterType;

	UPROPERTY(EditAnywhere)
	uint8 bCanAttack:1;

	UPROPERTY(EditAnywhere)
	uint8 bCanSprint:1;
	
	UPROPERTY(EditAnywhere)
	uint8 bFreeCameraAdjustment:1;
	
	UPROPERTY(EditAnywhere)
	uint8 bCanSwitchOut:1;

	UPROPERTY(EditAnywhere)
	FMovementProperties MovementProperties;
};

struct FAvailableInputs
{
	FAvailableInputs();
	FAvailableInputs(const FAvailableInputs& AvailableInputs);

	
	uint8 bCanAttack:1;
	uint8 bCanSprint:1;
	uint8 bFreeCameraAdjustment:1;
	uint8 bCanSwitchOut:1;
	FMovementProperties MovementProperties;

	//Handle for the limit reset timer
	FTimerHandle ResetHandle;
	
	//Set the limit for available inputs according to the given limits
	//If needed a reset time can be set, after which the limits will be reset automatically
	//if there are limits currently enacted, the according timer will be reset and after calling the timer
	//the state returns to the one before the first limits enacted
	bool LimitAvailableInputs(const FInputLimits& InputLimits, UWorld* World, float ResetTime = 0.f);

	//@return whether the given InputType is being limited at the moment
	bool CanOverrideCurrentInput(const EInputType InputType) const;

protected:
	//Used to save the old limits, so we can reset to them
	FInputLimits ResetToLimits;
	
	void CaptureCurrentLimits();
	void EnactLimits(const FInputLimits& InputLimits);
};