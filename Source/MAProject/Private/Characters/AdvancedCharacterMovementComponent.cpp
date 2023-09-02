// Fill out your copyright notice in the Description page of Project Settings.


#include "AdvancedCharacterMovementComponent.h"


// Sets default values for this component's properties
UAdvancedCharacterMovementComponent::UAdvancedCharacterMovementComponent() : bWalkBackwards(false)
{
}

FRotator UAdvancedCharacterMovementComponent::ComputeOrientToMovementRotation(const FRotator& CurrentRotation,
	float DeltaTime, FRotator& DeltaRotation) const
{
	FRotator Result = Super::ComputeOrientToMovementRotation(CurrentRotation, DeltaTime, DeltaRotation);
	if(bWalkBackwards)
	{
		DeltaRotation = (DeltaRotation - FRotator(180.f, 180.f, 180.f)).GetNormalized();
		Result = (DeltaRotation - FRotator(180.f, 180.f, 180.f)).GetNormalized();
	}
	return Result;
}

