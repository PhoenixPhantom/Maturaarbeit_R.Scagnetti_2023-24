// Fill out your copyright notice in the Description page of Project Settings.


#include "PositionalConstraint.h"

#include "NavigationSystem.h"
#include "Utility/Navigation/AvoidCharacterNavQueryFilter.h"

bool SampleGetClosestValid(FVector& ResultingLocation, const FVector& SourcePoint, const FVector& SpacedStartDirection,
                           float Distribution, const TArray<const FPositionalConstraint*>& RelevantConstraints,
                           float MaxSampleRange, UWorld* World, bool DebuggingEnabled)
{
	for(uint32 i = 1; true; i++)
	{
		FVector Direction = SpacedStartDirection * i;
		const double DirectionLength = Direction.Length();
		if(DirectionLength > MaxSampleRange) return false;
		
		const int32 Steps = round(DOUBLE_PI * 2.0 * DirectionLength / Distribution);
		const double RotationPerStep = Distribution/DirectionLength;

		FVector ProjectionExtent(Distribution/2.f, Distribution/2.f, 100.f);
		TArray<FVector> SamplePoints;
		UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetNavigationSystem(World);
		for(int32 j = 0; j < Steps; j++)
		{
			FNavLocation ProjectedLocation;
			if(NavigationSystem->ProjectPointToNavigation(SourcePoint +
				Direction.RotateAngleAxisRad(RotationPerStep * static_cast<double>(j),
					FVector(0.f, 0.f, 1.f)),ProjectedLocation, ProjectionExtent), 0,
					UAvoidCharacterNavQueryFilter::StaticClass())
						SamplePoints.Add(ProjectedLocation);
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
