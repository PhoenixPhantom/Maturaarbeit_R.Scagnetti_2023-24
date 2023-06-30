// Fill out your copyright notice in the Description page of Project Settings.


#include "PositionalConstraint.h"

#include "NavigationSystem.h"
#include "Components/BoxComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/Navigation/AvoidCharacterNavQueryFilter.h"

bool ShapeTraceMultiByProfile(UWorld* WorldContext, UShapeComponent* ShapeComponent, FName ProfileName, TArray<AActor*> ActorsToIgnore,
	TArray<FHitResult>& HitResults)
{
	if(ShapeComponent->IsA(USphereComponent::StaticClass()))
	{
		const USphereComponent* SphereComponent = CastChecked<USphereComponent>(ShapeComponent);
		return UKismetSystemLibrary::SphereTraceMultiByProfile(WorldContext, SphereComponent->GetComponentLocation(),
			SphereComponent->GetComponentLocation(), SphereComponent->GetScaledSphereRadius(), ProfileName,
			true, ActorsToIgnore, EDrawDebugTrace::None, HitResults, true);
	}
	else if(ShapeComponent->IsA(UBoxComponent::StaticClass()))
	{
		const UBoxComponent* BoxComponent = CastChecked<UBoxComponent>(ShapeComponent);
		return UKismetSystemLibrary::BoxTraceMultiByProfile(WorldContext, BoxComponent->GetComponentLocation(),
			BoxComponent->GetComponentLocation(), BoxComponent->GetScaledBoxExtent(),
			BoxComponent->GetComponentRotation(), ProfileName,
			true, ActorsToIgnore, EDrawDebugTrace::None, HitResults, true);
	}
	else unimplemented();
	return false;
}

bool SampleGetClosestValid(FVector& ResultingLocation, UShapeComponent* RequiredSpace, const FVector& SourcePoint,
	const FVector& SpacedStartDirection, float Distribution,
	const TArray<const FPositionalConstraint*>& RelevantConstraints, float MaxSampleRange, float ProjectionHalfHeight,
	UWorld* World,
	bool DebuggingEnabled)
{
	for(uint32 i = 1; true; i++)
	{
		FVector Direction = SpacedStartDirection * i;
		const double DirectionLength = Direction.Length();
		if(DirectionLength > MaxSampleRange) return false;
		
		const int32 Steps = round(DOUBLE_PI * 2.0 * DirectionLength / Distribution);
		const double RotationPerStep = Distribution/DirectionLength;

		FVector ProjectionExtent(Distribution/4.f, Distribution/4.f, ProjectionHalfHeight);
		TArray<FVector> SamplePoints;
		UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetNavigationSystem(World);
		for(int32 j = 0; j < Steps; j++)
		{
			FNavLocation ProjectedLocation;
			if(!NavigationSystem->ProjectPointToNavigation(SourcePoint +
				Direction.RotateAngleAxisRad(RotationPerStep * static_cast<double>(j),
					FVector(0.f, 0.f, 1.f)),ProjectedLocation, ProjectionExtent)) continue;
			TArray<FHitResult> HitResults;
			ShapeTraceMultiByProfile(World, RequiredSpace, "Navigation", {}, HitResults);
			bool IsValidLocation = true;
			for(const FHitResult HitResult : HitResults)
			{
				if(RequiredSpace->GetOwner() == HitResult.GetActor()) continue;
				IsValidLocation = false;
			}
			if(IsValidLocation) SamplePoints.Add(ProjectedLocation);
		}
		if(CheckSamplesForFirstValid(ResultingLocation, SamplePoints, RelevantConstraints, World,
			DebuggingEnabled)) return true;
	}
}

bool CheckSamplesForFirstValid(FVector& ValidPoint, const TArray<FVector>& SamplePoints,
                               const TArray<const FPositionalConstraint*>& RelevantConstraints, UWorld* World,
                               bool DebuggingEnabled)
{
	for(const FVector& SamplePoint : SamplePoints)
	{
		bool AreAllSatisfied = true;
		for(const FPositionalConstraint* Constraint : RelevantConstraints)
		{
			if(!Constraint->IsConstraintSatisfied(SamplePoint))
			{
				AreAllSatisfied = false;
				break;
			}
		}
#if WITH_EDITORONLY_DATA
		if(DebuggingEnabled)
		{
			if(AreAllSatisfied) DrawDebugPoint(World, SamplePoint + FVector(0.f, 0.f, 20.f), 10.f, FColor(0, 255, 0),
				false, 1, SDPG_World);
			else DrawDebugPoint(World, SamplePoint + FVector(0.f, 0.f, 20.f), 10.f, FColor(0, 0, 255),
				false, 0.1, SDPG_World);
		}
#endif
		
		if(AreAllSatisfied)
		{
			ValidPoint = SamplePoint;
			return true;
		}
	}
	return false;
}
