// Fill out your copyright notice in the Description page of Project Settings.


#include "AdvancedCharacterMovementComponent.h"


// Sets default values for this component's properties
UAdvancedCharacterMovementComponent::UAdvancedCharacterMovementComponent() : bWalkBackwards(false)
{
}

FRotator UAdvancedCharacterMovementComponent::ComputeOrientToMovementRotation(const FRotator& CurrentRotation,
	float DeltaTime, FRotator& DeltaRotation) const
{
	if (Acceleration.SizeSquared() < UE_KINDA_SMALL_NUMBER)
	{
		// AI path following request can orient us in that direction (it's effectively an acceleration)
		if (bHasRequestedVelocity && RequestedVelocity.SizeSquared() > UE_KINDA_SMALL_NUMBER)
		{
			return (bWalkBackwards ? -RequestedVelocity : RequestedVelocity).GetSafeNormal().Rotation();
		}

		// Don't change rotation if there is no acceleration.
		return CurrentRotation;
	}

	// Rotate toward direction of acceleration.
	return (bWalkBackwards ? -Acceleration : Acceleration).GetSafeNormal().Rotation();
}

