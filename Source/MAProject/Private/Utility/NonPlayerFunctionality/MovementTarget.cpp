// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/NonPlayerFunctionality/MovementTarget.h"

#include "AITypes.h"

// Sets default values
AMovementTarget::AMovementTarget() : BlendTime(1.f), TargetLocation(FAISystem::InvalidLocation), bForceNoInterpolation(false),
	bForceNoInterpolationOnce(false), TargetActor(nullptr), MaxVelocity(1000.f)
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ObjectRoot"));
	PrimaryActorTick.bCanEverTick = true;
}

// Called every frame
void AMovementTarget::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FVector Location = FAISystem::InvalidLocation;
	if(IsValid(TargetActor)) Location = TargetActor->GetActorLocation();
	else if (TargetLocation != FAISystem::InvalidLocation) Location = TargetLocation;

#if WITH_EDITORONLY_DATA
	if(bIsDebugging)
	{
		if(Location != FAISystem::InvalidLocation)
		{
			DrawDebugSphere(GetWorld(), Location, 50.f, 20,
				FColor(0, 0, 255), false, 0.f);
			DrawDebugSphere(GetWorld(), GetActorLocation(), 50.f, 20,
				FColor(0, 255, 255), false, 0.f);
		}
		else
		{
			DrawDebugSphere(GetWorld(), GetActorLocation(), 25.f, 20,
					FColor(255, 0, 255), false, 0.f);
		}
	}
#endif
	
	if(Location != FAISystem::InvalidLocation)
	{
		//Blend the object towards the target. Try to preserve velocity as good as possible to reduce jarring, but keep accuracy.
		const FVector Direction = Location - GetActorLocation();
		const FVector NecessaryVelocity = Direction/DeltaTime;
		const float ScaleBin = ceil(NecessaryVelocity.Length()/MaxVelocity);
		const FVector Velocity = NecessaryVelocity/ScaleBin;
		SetActorLocation(GetActorLocation() + Velocity * DeltaTime);
		
	}
}

void AMovementTarget::SetMovementTargetLocation(const FVector& NewTargetLocation, FSetMovementTargetKey Key)
{
	//if the location wasn't updated before, it makes no sense to interpolate to the target location
	if(TargetLocation == FAISystem::InvalidLocation && NewTargetLocation != FAISystem::InvalidLocation)
		SetActorLocation(NewTargetLocation);
	
	if(bForceNoInterpolationOnce || bForceNoInterpolation)
	{
		SetActorLocation(NewTargetLocation);
		bForceNoInterpolationOnce = false;
	}
	TargetLocation = NewTargetLocation;
}

