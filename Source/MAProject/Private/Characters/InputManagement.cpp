// Fill out your copyright notice in the Description page of Project Settings.


#include "InputManagement.h"

FInputLimits::FInputLimits() : LimiterType(EInputType::Attack), bCanAttack(true), bCanSprint(true),
	bFreeCameraAdjustment(true), bCanSwitchOut(true)
{
	MovementProperties.bCanCrouch = true;
	MovementProperties.bCanFly = true;
	MovementProperties.bCanJump = true;
	MovementProperties.bCanSwim = true;
	MovementProperties.bCanWalk = true;
}

bool FInputLimits::operator==(const FInputLimits& Compare) const
{
	//we have to do this manually as MovementProperties doesn't have a == operator
	return LimiterType == Compare.LimiterType && bCanSprint == Compare.bCanSprint &&
		bCanSwitchOut == Compare.bCanSwitchOut &&
		MovementProperties.bCanCrouch == Compare.MovementProperties.bCanCrouch &&
		MovementProperties.bCanFly == Compare.MovementProperties.bCanFly &&
		MovementProperties.bCanJump == Compare.MovementProperties.bCanJump &&
		MovementProperties.bCanSwim == Compare.MovementProperties.bCanSwim &&
		MovementProperties.bCanWalk == Compare.MovementProperties.bCanWalk;
}

FAvailableInputs::FAvailableInputs() : bCanAttack(true), bCanSprint(true),
	bFreeCameraAdjustment(true), bCanSwitchOut(true)
{
	MovementProperties.bCanJump = true;
	MovementProperties.bCanWalk = true;
	MovementProperties.bCanCrouch = true;
	MovementProperties.bCanFly = true;
	MovementProperties.bCanSwim = true;
	ResetToLimits.LimiterType = EInputType::Force; //the ResetToLimits should always be able to be set
}

FAvailableInputs::FAvailableInputs(const FAvailableInputs& AvailableInputs) : bCanAttack(AvailableInputs.bCanAttack),
	bCanSprint(AvailableInputs.bCanSprint),	bFreeCameraAdjustment(AvailableInputs.bFreeCameraAdjustment),
	bCanSwitchOut(AvailableInputs.bCanSwitchOut)
{
	ResetToLimits.LimiterType = EInputType::Force; //the ResetToLimits should always be able to be set
}

bool FAvailableInputs::LimitAvailableInputs(const FInputLimits& InputLimits, UWorld* World, float ResetTime)
{
	//limits can only be applied if they are issued by an input type that can now make changes
	if(!CanOverrideCurrentInput(InputLimits.LimiterType)) return false;
	check(IsValid(World));

	if(World->GetTimerManager().TimerExists(ResetHandle))
	{
		World->GetTimerManager().ClearTimer(ResetHandle);
		LimitAvailableInputs(ResetToLimits, World);
	}
	
	//Setting a timer with time == 0 doesn't work
	if(ResetTime > 0.f)
	{
		//Capture the old state, before setting the new one
		CaptureCurrentLimits();

		//Set a timer for the reset
		World->GetTimerManager().SetTimer(ResetHandle, [&, LocWorld = World]
		{
			LimitAvailableInputs(ResetToLimits, LocWorld);
		}, ResetTime, false);
	}
	EnactLimits(InputLimits);
	return true;
}

bool FAvailableInputs::CanOverrideCurrentInput(const EInputType InputType) const
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
		case EInputType::Force: { IsAllowed = true; break; } //Force can override everything
		default: { checkNoEntry(); }
	}
	return IsAllowed;
}

void FAvailableInputs::CaptureCurrentLimits()
{
	ResetToLimits.bCanAttack = bCanAttack;
	ResetToLimits.bCanSprint = bCanSprint;
	ResetToLimits.bCanSwitchOut = bCanSwitchOut;
	ResetToLimits.bFreeCameraAdjustment = bFreeCameraAdjustment;
	ResetToLimits.MovementProperties = MovementProperties;
}

void FAvailableInputs::EnactLimits(const FInputLimits& InputLimits)
{
	bCanAttack = InputLimits.bCanAttack;
	bCanSprint = InputLimits.bCanSprint;
	bCanSwitchOut = InputLimits.bCanSwitchOut;
	bFreeCameraAdjustment = InputLimits.bFreeCameraAdjustment;
	MovementProperties = InputLimits.MovementProperties;
}
