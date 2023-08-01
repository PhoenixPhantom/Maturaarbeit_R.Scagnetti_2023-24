// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputManagement.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInputLimitsResetDelegate, bool, bool&);

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
	//Death cannot be blocked or stopped from execution and by default does not allow for any input other than Death or Force
	//(the behavior can still be changed by manually initializing FInputLimits)
	Death,
	//Force cannot be blocked or stopped from execution, just like death, but by default it allows any input to follow up
	//(the behavior can still be changed by manually initializing FInputLimits)
	Force,
	//Used to end limits whose time cannot be a predetermined value
	//Will always be executed for limits without predetermined ending (resets everything)
	//and has no effect otherwise (doesn't set anything)
	//(the behavior of FInputLimits using Reset CANNOT be changed through changing ANY values in FInputLimits)
	Reset
	
};

bool operator==(const FMovementProperties& Comp1, const FMovementProperties& Comp2);

USTRUCT()
struct FInputLimits
{
	GENERATED_BODY()

	FInputLimits();
	FInputLimits(const EInputType Input);
	FInputLimits(const EInputType Input, bool SetAll);
	FInputLimits(const EInputType Input, float LimitationTime);
	FInputLimits(const EInputType Input, float LimitationTime, const struct FAcceptedInputs& AcceptedInputs);
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
	
	FOnInputLimitsResetDelegate OnInputLimitsReset;
	
	//Limit available inputs according to the given parameters for the given time. After the time has passed,
	//the state returns to what it was before the first limits enacted
	bool LimitAvailableInputs(const FInputLimits& InputLimits, UWorld* World);

	//Limit available inputs according to the given parameters for the given time.
	//Follows up with the next limit if no other limit has been enforced since the first one.
	//After enacting both limits for their respective time,
	//the state returns to what it was before the first limits enacted
	//bool LimitAvailableInputs(const FInputLimits& FirstLimits, const FInputLimits& SecondLimits, UWorld* World);

	//@return whether the given InputType is being limited at the moment
	bool CanOverrideCurrentInput(const EInputType InputType) const;
	
	void SetDefaultLimits(const FInputLimits& NewDefaultLimits);

protected:
	//Used to save the old limits, so we can reset to them
	FInputLimits DefaultLimits;

	void ResetLimits(UWorld* World, bool IsLimitDurationOver = false);
	bool IsAlreadyReset() const;
	void EnactLimits(const FInputLimits& InputLimits);
};
