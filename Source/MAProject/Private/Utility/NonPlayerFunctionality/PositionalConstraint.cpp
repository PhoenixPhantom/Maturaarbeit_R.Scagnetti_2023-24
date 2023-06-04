// Fill out your copyright notice in the Description page of Project Settings.


#include "PositionalConstraint.h"

#include "NavigationSystem.h"
#include "Kismet/KismetSystemLibrary.h"

bool SampleGetClosestValid(FVector& ResultingLocation, const FVector& SourcePoint, const FVector& SpacedStartDirection,
                           float Distribution, const TArray<const FPositionalConstraint*>& RelevantConstraints,
                           float MaxSampleRange, UWorld* World, bool DebuggingEnabled)
{
	for(uint32 i = 1; true; i++)
	{
		FVector Direction = SpacedStartDirection * i;
		const double DirectionLength = Direction.Length();
		if(DirectionLength > MaxSampleRange) return false;

/*#if WITH_EDITORONLY_DATA
		if(DebuggingEnabled)
		{
			UKismetSystemLibrary::DrawDebugCircle(World, SourcePoint, DirectionLength, 20,
				FLinearColor(0, 0, 255), 0, 5.f, FVector(1, 0, 0),
				FVector(0, 1, 0), true);
		}
#endif*/
		const double RadiusDistance = DOUBLE_PI * 2.0 * DirectionLength;
		const int32 Steps = std::round(RadiusDistance / Distribution);
		const double RotationPerStep = Distribution/RadiusDistance;

		TArray<FVector> SamplePoints;
		UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetNavigationSystem(World);
		for(int32 j = 1; j < Steps; j++)
		{
			FNavLocation ProjectedLocation;
			if(NavigationSystem->ProjectPointToNavigation(SourcePoint + Direction, ProjectedLocation,
				FVector(Distribution/2.f))) SamplePoints.Add(ProjectedLocation);
			Direction.RotateAngleAxisRad(RotationPerStep, FVector(0.f, 0.f, 1.f));
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
			GLog->Log("Print");
			if(AreAllSatisfied) DrawDebugPoint(World, SamplePoint, 10.f, FColor(0, 0, 255),
				false, 0.1, SDPG_Foreground);
			else DrawDebugPoint(World, SamplePoint, 10.f, FColor(0, 255, 0), false, 0.1, SDPG_Foreground);
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
