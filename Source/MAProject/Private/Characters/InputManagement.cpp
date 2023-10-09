// Fill out your copyright notice in the Description page of Project Settings.


#include "InputManagement.h"

bool operator==(const FMovementProperties& Comp1, const FMovementProperties& Comp2)
{
	return Comp1.bCanCrouch == Comp2.bCanCrouch && Comp1.bCanFly == Comp2.bCanFly && Comp1.bCanJump == Comp2.bCanJump &&
		Comp1.bCanSwim == Comp2.bCanSwim && Comp1.bCanWalk == Comp2.bCanWalk;
}

FInputLimits::FInputLimits() : LimiterType(EInputType::Force), LimitationDuration(0.f), bCanAttack(true),
                               bCanGetStaggered(true), bCanRun(true), bFreeCameraAdjustment(true), bCanSwitchOut(true)
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
	case EInputType::Jump:
		{
			bCanRun = bCanAttack = bCanSwitchOut  = MovementProperties.bCanJump =
				MovementProperties.bCanSwim = MovementProperties.bCanCrouch = false;
			bFreeCameraAdjustment = bCanGetStaggered = MovementProperties.bCanFly = MovementProperties.bCanWalk = true;
			break;
		}
		case EInputType::Attack:
			{
				bCanRun = bCanAttack = bCanSwitchOut = MovementProperties.bCanFly = MovementProperties.bCanJump =
					MovementProperties.bCanSwim = MovementProperties.bCanWalk = false;
				bFreeCameraAdjustment = MovementProperties.bCanCrouch = bCanGetStaggered = true;
				break;
			}
		case EInputType::Stagger:
			{
				bCanRun = bCanAttack = bCanSwitchOut = MovementProperties.bCanFly = MovementProperties.bCanJump =
					MovementProperties.bCanSwim = MovementProperties.bCanWalk = MovementProperties.bCanCrouch = false;
				bFreeCameraAdjustment = bCanGetStaggered = true;
				break;
			}
		case EInputType::Death:
			{
				bCanRun = bCanAttack = bCanSwitchOut = bFreeCameraAdjustment = MovementProperties.bCanFly =
					MovementProperties.bCanJump = MovementProperties.bCanSwim = MovementProperties.bCanWalk =
					MovementProperties.bCanCrouch = bCanGetStaggered = false;
				break;
			}
	case EInputType::Force:
			{
				bCanRun = bCanAttack = bCanSwitchOut = bFreeCameraAdjustment = MovementProperties.bCanFly =
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

FInputLimits::FInputLimits(const EInputType Input, bool SetAll) : LimiterType(Input), LimitationDuration(0.f)
{
	bCanRun = bCanAttack = bCanSwitchOut = bFreeCameraAdjustment = MovementProperties.bCanFly =
					MovementProperties.bCanJump = MovementProperties.bCanSwim = MovementProperties.bCanWalk =
					MovementProperties.bCanCrouch = bCanGetStaggered = SetAll;
}

FInputLimits::FInputLimits(const EInputType Input, float LimitationTime) : FInputLimits(Input)
{
	LimitationDuration = LimitationTime;
}

FInputLimits::FInputLimits(const EInputType Input, float LimitationTime, const FAcceptedInputs& AcceptedInputs) :
	LimiterType(Input), LimitationDuration(LimitationTime), bCanAttack(AcceptedInputs.bCanAttack),
	bCanGetStaggered(AcceptedInputs.bCanGetStaggered), bCanRun(AcceptedInputs.bCanRun),
	bFreeCameraAdjustment(AcceptedInputs.bFreeCameraAdjustment), bCanSwitchOut(AcceptedInputs.bCanSwitchOut),
	MovementProperties(AcceptedInputs.MovementProperties)
{}

bool FInputLimits::operator==(const FInputLimits& Compare) const
{
	//we have to do this manually as MovementProperties doesn't have a == operator
	return LimiterType == Compare.LimiterType && LimitationDuration == Compare.LimitationDuration &&
		bCanAttack == Compare.bCanAttack && bCanGetStaggered == Compare.bCanGetStaggered &&
		bCanRun == Compare.bCanRun && bCanSwitchOut == Compare.bCanSwitchOut &&
		MovementProperties == Compare.MovementProperties;
}

FAcceptedInputs::FAcceptedInputs() : bCanAttack(true), bCanGetStaggered(true), bCanRun(true),
	bFreeCameraAdjustment(true), bCanSwitchOut(true), DefaultLimits(EInputType::Force, true)
{
	MovementProperties.bCanJump = true;
	MovementProperties.bCanWalk = true;
	MovementProperties.bCanCrouch = true;
	MovementProperties.bCanFly = true;
	MovementProperties.bCanSwim = true;
}

FAcceptedInputs::FAcceptedInputs(const FAcceptedInputs& AvailableInputs) : bCanAttack(AvailableInputs.bCanAttack),
            bCanGetStaggered(AvailableInputs.bCanGetStaggered), bCanRun(AvailableInputs.bCanRun),
            bFreeCameraAdjustment(AvailableInputs.bFreeCameraAdjustment), bCanSwitchOut(AvailableInputs.bCanSwitchOut),
			DefaultLimits(EInputType::Force)
{
}

bool FAcceptedInputs::LimitAvailableInputs(const FInputLimits& InputLimits, UWorld* World)
{
	//if we have a limit without timer, we can reset the limits by using EInputType::Reset
	if(InputLimits.LimiterType == EInputType::Reset && !World->GetTimerManager().TimerExists(ResetHandle))
	{
		ResetLimits(World, true);
		return true;
	}
	
	//limits can only be applied if they are issued by an input type that can now make changes
	if(!CanOverrideCurrentInput(InputLimits.LimiterType)) return false;

	ResetLimits(World);
	
	//Setting a timer with time == 0.f doesn't work
	if(InputLimits.LimitationDuration > 0.f)
	{	
		//Set a timer for the reset
		World->GetTimerManager().SetTimer(ResetHandle, [this, World]()
		{
			ResetLimits(World, true);
		}, InputLimits.LimitationDuration, false);
	}
	EnactLimits(InputLimits);
	return true;
}

bool FAcceptedInputs::CanOverrideCurrentInput(const EInputType InputType) const
{
	bool IsAllowed = false;
	switch(InputType)
	{
		case EInputType::Walk: { IsAllowed = MovementProperties.bCanWalk; break; }
		case EInputType::Sprint: { IsAllowed = bCanRun; break; }
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

void FAcceptedInputs::SetDefaultLimits(const FInputLimits& NewDefaultLimits)
{
	DefaultLimits.bCanAttack = bCanAttack;
	DefaultLimits.bCanGetStaggered = bCanGetStaggered;
	DefaultLimits.bCanRun = bCanRun;
	DefaultLimits.bCanSwitchOut = bCanSwitchOut;
	DefaultLimits.bFreeCameraAdjustment = bFreeCameraAdjustment;
	DefaultLimits.MovementProperties = MovementProperties;
}

void FAcceptedInputs::ResetLimits(UWorld* World, bool IsLimitDurationOver)
{
	if(IsAlreadyReset())
	{
		return;
	}
	check(IsValid(World));
	if(World->GetTimerManager().TimerExists(ResetHandle))
	{
		World->GetTimerManager().ClearTimer(ResetHandle);
	}
	EnactLimits(DefaultLimits);

	//copy the execution stack, since some functions bound to OnInputLimitsReset bind new ones to OnInputLimitsReset 
	const FOnInputLimitsResetDelegate ExecutionStack = OnInputLimitsReset;
	//the on reset functions, in general, are specific to this limit and will not be called the next time
	OnInputLimitsReset.Clear();
	
	ExecutionStack.Broadcast(IsLimitDurationOver);
}

bool FAcceptedInputs::IsAlreadyReset() const
{
	return DefaultLimits.bCanAttack == bCanAttack && DefaultLimits.bCanGetStaggered == bCanGetStaggered &&
		DefaultLimits.bCanRun == bCanRun && DefaultLimits.bCanSwitchOut == bCanSwitchOut &&
		DefaultLimits.bFreeCameraAdjustment == bFreeCameraAdjustment &&
		DefaultLimits.MovementProperties == MovementProperties;
}

void FAcceptedInputs::EnactLimits(const FInputLimits& InputLimits)
{
	bCanAttack = InputLimits.bCanAttack;
	bCanGetStaggered = InputLimits.bCanGetStaggered;
	bCanRun = InputLimits.bCanRun;
	bCanSwitchOut = InputLimits.bCanSwitchOut;
	bFreeCameraAdjustment = InputLimits.bFreeCameraAdjustment;
	MovementProperties = InputLimits.MovementProperties;
}
