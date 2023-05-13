// Fill out your copyright notice in the Description page of Project Settings.


#include "InputManagement.h"

FInputLimits::FInputLimits() : LimiterType(EInputType::Force), LimitationDuration(0.f), bCanAttack(true),
	bCanGetStaggered(true), bCanSprint(true), bFreeCameraAdjustment(true), bCanSwitchOut(true)
{
	MovementProperties.bCanCrouch = true;
	MovementProperties.bCanFly = true;
	MovementProperties.bCanJump = true;
	MovementProperties.bCanSwim = true;
	MovementProperties.bCanWalk = true;
}

FInputLimits::FInputLimits(const EInputType Input) : LimiterType(Input), LimitationDuration(0.f)
{
	switch(LimiterType)
	{
		case EInputType::Attack:
			{
				bCanSprint = bCanAttack = bCanSwitchOut = MovementProperties.bCanFly = MovementProperties.bCanJump =
					MovementProperties.bCanSwim = MovementProperties.bCanWalk = false;
				bFreeCameraAdjustment = MovementProperties.bCanCrouch = bCanGetStaggered = true;
				break;
			}
		case EInputType::Stagger:
			{
				bCanSprint = bCanAttack = bCanSwitchOut = MovementProperties.bCanFly = MovementProperties.bCanJump =
					MovementProperties.bCanSwim = MovementProperties.bCanWalk = MovementProperties.bCanCrouch = false;
				bFreeCameraAdjustment = bCanGetStaggered = true;
				break;
			}
		case EInputType::Death:
			{
				bCanSprint = bCanAttack = bCanSwitchOut = bFreeCameraAdjustment = MovementProperties.bCanFly =
					MovementProperties.bCanJump = MovementProperties.bCanSwim = MovementProperties.bCanWalk =
					MovementProperties.bCanCrouch = bCanGetStaggered = false;
				break;
			}
	case EInputType::Force:
			{
				bCanSprint = bCanAttack = bCanSwitchOut = bFreeCameraAdjustment = MovementProperties.bCanFly =
					MovementProperties.bCanJump = MovementProperties.bCanSwim = MovementProperties.bCanWalk =
					MovementProperties.bCanCrouch = bCanGetStaggered = true;
			}
	case EInputType::Reset:
			{
				//other settings have no effect when EInputType == Reset
				break;
			}
		default:
			unimplemented();
	}
}

FInputLimits::FInputLimits(const EInputType Input, float LimitationTime, const FAcceptedInputs& AcceptedInputs) :
	LimiterType(Input), LimitationDuration(LimitationTime), bCanAttack(AcceptedInputs.bCanAttack),
	bCanGetStaggered(AcceptedInputs.bCanGetStaggered), bCanSprint(AcceptedInputs.bCanSprint),
	bFreeCameraAdjustment(AcceptedInputs.bFreeCameraAdjustment), bCanSwitchOut(AcceptedInputs.bCanSwitchOut),
	MovementProperties(AcceptedInputs.MovementProperties)
{}

bool FInputLimits::operator==(const FInputLimits& Compare) const
{
	//we have to do this manually as MovementProperties doesn't have a == operator
	return LimiterType == Compare.LimiterType && LimitationDuration == Compare.LimitationDuration &&
		bCanAttack == Compare.bCanAttack && bCanGetStaggered == Compare.bCanGetStaggered &&
		bCanSprint == Compare.bCanSprint && bCanSwitchOut == Compare.bCanSwitchOut &&
		MovementProperties.bCanCrouch == Compare.MovementProperties.bCanCrouch &&
		MovementProperties.bCanFly == Compare.MovementProperties.bCanFly &&
		MovementProperties.bCanJump == Compare.MovementProperties.bCanJump &&
		MovementProperties.bCanSwim == Compare.MovementProperties.bCanSwim &&
		MovementProperties.bCanWalk == Compare.MovementProperties.bCanWalk;
}

FAcceptedInputs::FAcceptedInputs() : bCanAttack(true), bCanGetStaggered(true), bCanSprint(true),
	bFreeCameraAdjustment(true), bCanSwitchOut(true)
{
	MovementProperties.bCanJump = true;
	MovementProperties.bCanWalk = true;
	MovementProperties.bCanCrouch = true;
	MovementProperties.bCanFly = true;
	MovementProperties.bCanSwim = true;
	ResetToLimits.LimiterType = EInputType::Force; //the ResetToLimits should always be able to be set
}

FAcceptedInputs::FAcceptedInputs(const FAcceptedInputs& AvailableInputs) : bCanAttack(AvailableInputs.bCanAttack),
                                                                           bCanGetStaggered(AvailableInputs.bCanGetStaggered), bCanSprint(AvailableInputs.bCanSprint),
                                                                           bFreeCameraAdjustment(AvailableInputs.bFreeCameraAdjustment), bCanSwitchOut(AvailableInputs.bCanSwitchOut)
{
	ResetToLimits.LimiterType = EInputType::Force; //the ResetToLimits should always be able to be set
}

bool FAcceptedInputs::LimitAvailableInputs(const FInputLimits& InputLimits, UWorld* World)
{
	//if we have a limit without timer, we can reset the limits by using EInputType::Reset
	if(InputLimits.LimiterType == EInputType::Reset && !World->GetTimerManager().TimerExists(ResetHandle))
	{
		EnactLimits(ResetToLimits);
		return true;
	}
	//limits can only be applied if they are issued by an input type that can now make changes
	if(!CanOverrideCurrentInput(InputLimits.LimiterType)) return false;

	ResetLimits(World);
	//Setting a timer with time == 0.f doesn't work
	if(InputLimits.LimitationDuration > 0.f)
	{
		//Capture the old state, before setting the new one
		CaptureCurrentLimits();

		//Set a timer for the reset
		World->GetTimerManager().SetTimer(ResetHandle, [this, World]()
		{
			LimitAvailableInputs(ResetToLimits, World);
		}, InputLimits.LimitationDuration, false);
	}
	EnactLimits(InputLimits);
	return true;
}

bool FAcceptedInputs::LimitAvailableInputs(const FInputLimits& FirstLimits, const FInputLimits& SecondLimits,
                                           UWorld* World)
{
	//limits can only be applied if they are issued by an input type that can now make changes
	if(!CanOverrideCurrentInput(FirstLimits.LimiterType)) return false;
	
	ResetLimits(World);
	//Setting a timer with time == 0.f doesn't work
	if(FirstLimits.LimitationDuration > 0.f)
	{
		//Capture the old state, before setting the new one
		CaptureCurrentLimits();
		
		//Set a timer for the reset
		World->GetTimerManager().SetTimer(ResetHandle, [this, SecondLimits, World]()
		{
			if(!CanOverrideCurrentInput(SecondLimits.LimiterType)) LimitAvailableInputs(ResetToLimits, World);
			LimitAvailableInputs(SecondLimits, World);
		}, FirstLimits.LimitationDuration, false);
	}
	EnactLimits(FirstLimits);
	return true;
}

bool FAcceptedInputs::CanOverrideCurrentInput(const EInputType InputType) const
{
	bool IsAllowed = false;
	switch(InputType)
	{
		case EInputType::Walk: { IsAllowed = MovementProperties.bCanWalk; break; }
		case EInputType::Sprint: { IsAllowed = bCanSprint; break; }
		case EInputType::Camera: { IsAllowed = bFreeCameraAdjustment; break; }
		case EInputType::Jump: { IsAllowed = MovementProperties.bCanJump; break; }
		case EInputType::Attack: { IsAllowed = bCanAttack; break; }
		case EInputType::SwitchOut: { IsAllowed = bCanSwitchOut; break;}
		case EInputType::Stagger: { IsAllowed = bCanGetStaggered; break; }
		case EInputType::Reset: { IsAllowed = false;  break; }
		case EInputType::Force: case EInputType::Death: { IsAllowed = true; break; } //These types can override everything
		default: { checkNoEntry(); }
	}
	return IsAllowed;
}

void FAcceptedInputs::ResetLimits(UWorld* World)
{
	check(IsValid(World));
	if(World->GetTimerManager().TimerExists(ResetHandle))
	{
		World->GetTimerManager().ClearTimer(ResetHandle);
		LimitAvailableInputs(ResetToLimits, World);
	}
}

void FAcceptedInputs::CaptureCurrentLimits()
{
	ResetToLimits.bCanAttack = bCanAttack;
	ResetToLimits.bCanGetStaggered = bCanGetStaggered;
	ResetToLimits.bCanSprint = bCanSprint;
	ResetToLimits.bCanSwitchOut = bCanSwitchOut;
	ResetToLimits.bFreeCameraAdjustment = bFreeCameraAdjustment;
	ResetToLimits.MovementProperties = MovementProperties;
}

void FAcceptedInputs::EnactLimits(const FInputLimits& InputLimits)
{
	bCanAttack = InputLimits.bCanAttack;
	bCanGetStaggered = InputLimits.bCanGetStaggered;
	bCanSprint = InputLimits.bCanSprint;
	bCanSwitchOut = InputLimits.bCanSwitchOut;
	bFreeCameraAdjustment = InputLimits.bFreeCameraAdjustment;
	MovementProperties = InputLimits.MovementProperties;
}
