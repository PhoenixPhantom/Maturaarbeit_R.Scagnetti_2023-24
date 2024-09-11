// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputManagement.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnInputLimitsResetDelegate, bool);

UENUM(meta=(Bitflags, UseEnumValuesAsMaskValuesInEditor="true"))
enum class EInputType : int32
{
	Undefined		= 0 UMETA(Hidden), //should only be used as default value for certainly changed values
	Walk			= 1 << 0,
	Sprint			= 1 << 1,
	Camera			= 1 << 2,
	Jump			= 1 << 3,
	Attack			= 1 << 4,
	SwitchOut		= 1 << 5,
	Stagger			= 1 << 6,
	HeavyStagger	= 1 << 7,
	//Death cannot be blocked or stopped from execution and by default does not allow for any input other than Death or Force
	//(the behavior can still be changed by manually initializing FInputLimits)
	Death			= 1 << 8 UMETA(Hidden), 
	//Force cannot be blocked or stopped from execution, just like death, but by default it allows any input to follow up
	//(the behavior can still be changed by manually initializing FInputLimits)
	Force			= 1 << 9 UMETA(Hidden),
	//Used to end limits whose time cannot be a predetermined value
	//Will always be executed for limits without predetermined ending (resets everything)
	//and has no effect otherwise (doesn't set anything)
	//(the behavior of FInputLimits using Reset CANNOT be changed through changing ANY values in FInputLimits)
	Reset			= 1 << 10 UMETA(Hidden)	
};
ENUM_CLASS_FLAGS(EInputType)


bool operator==(const FMovementProperties& Comp1, const FMovementProperties& Comp2);

USTRUCT()
struct FNewInputLimits
{
	GENERATED_BODY()

	FNewInputLimits();
	FNewInputLimits(const EInputType Input);
	FNewInputLimits(const EInputType Input, float LimitationTime);
	FNewInputLimits(const FNewInputLimits& Limits);

	bool operator==(const FNewInputLimits& Compare) const;
	void AddAllowedInputs(const EInputType AdditionalLimits);
	bool IsAllowedInput(const EInputType InputType) const { return AllowedInputs & static_cast<int32>(InputType); };
	
	///@brief The type of input that these limits are applied by
	UPROPERTY(EditAnywhere)
	EInputType LimiterType;

	UPROPERTY(EditAnywhere, meta=(ForceUnits="s", ToolTip="The time for which the limitation will be in place (0 means that no limitation will be put in place)"))
	float LimitationDuration;

	UPROPERTY(EditAnywhere, meta=(Bitmask, BitmaskEnum = "/Script/MAProject.EInputType"))
	int32 AllowedInputs;
};

bool operator==(const TDelegate<void(bool)>& a, const TDelegate<void(bool)>& b);

struct FAcceptedInputs
{
	FAcceptedInputs();
	FAcceptedInputs(const FAcceptedInputs& AvailableInputs);

	int32 AllowedInputs;

	void AddAllowedInputType(EInputType InputType){ AllowedInputs |= static_cast<int32>(InputType); };

	//Handle for the limit reset timer
	FTimerHandle ResetHandle;

	//for some random reason UE5's multicast delegate stores the state of
	//the caller object wrongly, so we can't use that one here
	TArray<TDelegate<void(bool)>> OnInputLimitsReset;
	
	//Limit available inputs according to the given parameters for the given time. After the time has passed,
	//the state returns to what it was before the first limits enacted
	bool LimitAvailableInputs(const FNewInputLimits& InputLimits, UWorld* World);

	//Limit available inputs according to the given parameters for the given time.
	//Follows up with the next limit if no other limit has been enforced since the first one.
	//After enacting both limits for their respective time,
	//the state returns to what it was before the first limits enacted
	//bool LimitAvailableInputs(const FInputLimits& FirstLimits, const FInputLimits& SecondLimits, UWorld* World);

	//@return whether the given InputType is being limited at the moment
	bool IsAllowedInput(const EInputType InputType) const;

	void ResetLimits(UWorld* World, bool IsLimitDurationOver = false);
	bool IsAlreadyReset() const;
};
