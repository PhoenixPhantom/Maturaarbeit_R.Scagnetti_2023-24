// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/Animation/CustomAnimInstance.h"

UCustomAnimInstance::UCustomAnimInstance(): MovementSpeed(0.f), LegIKBlend(0.f), DeathAnimTime(0.25f), bIsDying(false),
	bIsInAir(false), bIsInCustomState0(false), bIsInCustomState1(false), bIsInCustomState2(false),
	bAllowLeftFrontLeg(true), bAllowRightFrontLeg(true), bAllowLeftBackLeg(true), bAllowRightBackLeg(true)
{
}

void UCustomAnimInstance::SetMovement(const FVector& MovementVector)
{
	MovementSpeed = MovementVector.Length();
	RelativeMovementDirection = GetOwningActor()->GetActorRotation().UnrotateVector(MovementVector);
}

bool UCustomAnimInstance::IsInState(ECustomAnimationState State) const
{
	switch(State)
	{
	case ECustomAnimationState::CustomState0:
		{
			return bIsInCustomState0;
		}
	case ECustomAnimationState::CustomState1:
		{
			return bIsInCustomState1;
		}
	case ECustomAnimationState::CustomState2:
		{
			return bIsInCustomState2;
		}
	default:
		{
			checkNoEntry();
			return false;
		}
	}
}

void UCustomAnimInstance::SetAllowedLegIKTypes(int32 AllowedTypes, float Duration)
{
	bAllowLeftFrontLeg = (AllowedTypes >> 0) & 0b1;
	bAllowRightFrontLeg = (AllowedTypes >> 1) & 0b1;
	bAllowLeftBackLeg = (AllowedTypes >> 2) & 0b1;
	bAllowRightBackLeg = (AllowedTypes >> 3) & 0b1;
	GetWorld()->GetTimerManager().SetTimer(AllowedIKTypesResetHandle, [Local = this]()
	{
		if(!IsValid(Local)) return;
		Local->bAllowLeftFrontLeg = Local->bAllowRightFrontLeg = Local->bAllowLeftBackLeg = Local->bAllowRightBackLeg = true;
	}, Duration, false);	
}

void UCustomAnimInstance::EnterCustomState(ECustomAnimationState TargetState)
{
	switch(TargetState)
	{
		case ECustomAnimationState::CustomState0:
		{
			bIsInCustomState0 = true;
			break;
		}
		case ECustomAnimationState::CustomState1:
		{
			bIsInCustomState1 = true;
			break;
		}
		case ECustomAnimationState::CustomState2:
		{
			bIsInCustomState2 = true;
			break;
		}
	default: checkNoEntry();
	}
}

void UCustomAnimInstance::ExitCustomState(ECustomAnimationState TargetState)
{
	switch(TargetState)
	{
	case ECustomAnimationState::CustomState0:
		{
			bIsInCustomState0 = false;
			break;
		}
	case ECustomAnimationState::CustomState1:
		{
			bIsInCustomState1 = false;
			break;
		}
	case ECustomAnimationState::CustomState2:
		{
			bIsInCustomState2 = false;
			break;
		}
	default: checkNoEntry();
	}
}
