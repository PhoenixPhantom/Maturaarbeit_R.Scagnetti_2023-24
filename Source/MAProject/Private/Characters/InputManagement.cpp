// Fill out your copyright notice in the Description page of Project Settings.


#include "InputManagement.h"

FNewInputLimits::FNewInputLimits(): LimiterType(EInputType::Force), LimitationDuration(0.f), AllowedInputs(~0){}

FNewInputLimits::FNewInputLimits(const EInputType Input): LimiterType(Input), LimitationDuration(1.f), AllowedInputs(0)
{
	switch(LimiterType)
	{
	case EInputType::Jump:
		{
			AddAllowedInputs(EInputType::Camera | EInputType::Stagger | EInputType::HeavyStagger |
				EInputType::Walk); //walk is used for in-air-control here
			break;
		}
	case EInputType::Attack:
		{
			AddAllowedInputs(EInputType::Camera | EInputType::Stagger | EInputType::HeavyStagger);
			break;
		}
	case EInputType::Stagger:
		{
			AddAllowedInputs(EInputType::Camera | EInputType::Stagger | EInputType::HeavyStagger);
			break;
		}
	case EInputType::HeavyStagger:
		{
			AddAllowedInputs(EInputType::Camera);
			break;
		}
	case EInputType::Death:
		{
			//no other inputs allowed
			break;
		}
	case EInputType::Force:
		{
			//no other inputs allowed
			break;
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

FNewInputLimits::FNewInputLimits(const EInputType Input, float LimitationTime) : FNewInputLimits(Input)
{
	LimitationDuration = LimitationTime;
}

FNewInputLimits::FNewInputLimits(const FNewInputLimits& Limits) : LimiterType(Limits.LimiterType),
	LimitationDuration(Limits.LimitationDuration), AllowedInputs(Limits.AllowedInputs)
{
}

bool FNewInputLimits::operator==(const FNewInputLimits& Compare) const
{
	return LimiterType == Compare.LimiterType && LimitationDuration == Compare.LimitationDuration &&
		AllowedInputs == Compare.AllowedInputs;
}

void FNewInputLimits::AddAllowedInputs(const EInputType AdditionalLimits)
{
	AllowedInputs |= static_cast<int32>(AdditionalLimits);
}

bool operator==(const TDelegate<void(bool)>& a, const TDelegate<void(bool)>& b)
{
	return a.GetHandle() == b.GetHandle();
}

FAcceptedInputs::FAcceptedInputs() : AllowedInputs(~0)
{
}

FAcceptedInputs::FAcceptedInputs(const FAcceptedInputs& AvailableInputs) : AllowedInputs(AvailableInputs.AllowedInputs)
{
}

bool FAcceptedInputs::LimitAvailableInputs(const FNewInputLimits& InputLimits, UWorld* World)
{
	//if we have a limit without timer, we can reset the limits by using EInputType::Reset
	if(InputLimits.LimiterType == EInputType::Reset && !World->GetTimerManager().TimerExists(ResetHandle))
	{
		ResetLimits(World, true);
		return true;
	}
	
	//limits can only be applied if they are issued by an input type that can now make changes
	if(!IsAllowedInput(InputLimits.LimiterType)) return false;

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
	AllowedInputs = InputLimits.AllowedInputs;
	return true;
}

bool FAcceptedInputs::IsAllowedInput(const EInputType InputType) const
{
	if(InputType == EInputType::Force || InputType == EInputType::Death) return !(!AllowedInputs);
	return AllowedInputs & static_cast<int32>(InputType);
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

	//copy the execution stack, since some functions bound to OnInputLimitsReset bind new ones to OnInputLimitsReset 
	const TArray<TDelegate<void(bool)>> ExecutionStack = OnInputLimitsReset;
	//the on reset functions, in general, are specific to this limit and will not be called the next time
	OnInputLimitsReset.Empty();
	AllowedInputs = ~0;

	for(TDelegate<void(bool)> Delegate : ExecutionStack)
	{
		// ReSharper disable once CppExpressionWithoutSideEffects
		Delegate.ExecuteIfBound(IsLimitDurationOver);
	}
}

bool FAcceptedInputs::IsAlreadyReset() const
{
	return AllowedInputs == ~0; //= all
}
