// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	Stagger,
	Death,
	//Force is the strongest input type and cannot be blocked or stopped from execution
	Force
	
};

USTRUCT()
struct FInputLimits
{
	GENERATED_BODY()

	FInputLimits();
	FInputLimits(const EInputType Input);
	FInputLimits(const FInputLimits& Limits) = default;

	bool operator==(const FInputLimits& Compare) const;

	///@brief The type of input that these limits are applied by
	UPROPERTY(EditAnywhere)
	EInputType LimiterType;

	UPROPERTY(EditAnywhere, meta=(ForceUnits="s", ToolTip="The time for which the limitation will be in place (0 means that no limitation will be put in place)"))
	float LimitationDuration;

	UPROPERTY(EditAnywhere)
	uint8 bCanAttack:1;

	UPROPERTY(EditAnywhere)
	uint8 bCanGetStaggered:1;

	UPROPERTY(EditAnywhere)
	uint8 bCanSprint:1;
	
	UPROPERTY(EditAnywhere)
	uint8 bFreeCameraAdjustment:1;
	
	UPROPERTY(EditAnywhere)
	uint8 bCanSwitchOut:1;

	UPROPERTY(EditAnywhere)
	FMovementProperties MovementProperties;
};

struct FAcceptedInputs
{
	FAcceptedInputs();
	FAcceptedInputs(const FAcceptedInputs& AvailableInputs);

	
	uint8 bCanAttack:1;
	uint8 bCanGetStaggered:1;
	uint8 bCanSprint:1;
	uint8 bFreeCameraAdjustment:1;
	uint8 bCanSwitchOut:1;
	FMovementProperties MovementProperties;

	//Handle for the limit reset timer
	FTimerHandle ResetHandle;
	
	//Limit available inputs according to the given parameters for the given time. After the time has passed,
	//the state returns to what it was before the first limits enacted
	bool LimitAvailableInputs(const FInputLimits& InputLimits, UWorld* World);

	//Limit available inputs according to the given parameters for the given time.
	//Follows up with the next limit if no other limit has been enforced since the first one.
	//After enacting both limits for their respective time,
	//the state returns to what it was before the first limits enacted
	bool LimitAvailableInputs(const FInputLimits& FirstLimits, const FInputLimits& SecondLimits, UWorld* World);

	//@return whether the given InputType is being limited at the moment
	bool CanOverrideCurrentInput(const EInputType InputType) const;

protected:
	//Used to save the old limits, so we can reset to them
	FInputLimits ResetToLimits;

	void ResetLimits(UWorld* World);
	void CaptureCurrentLimits();
	void EnactLimits(const FInputLimits& InputLimits);
};
