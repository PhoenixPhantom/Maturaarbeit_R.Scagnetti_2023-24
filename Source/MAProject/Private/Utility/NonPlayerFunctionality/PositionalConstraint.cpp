// Fill out your copyright notice in the Description page of Project Settings.


#include "PositionalConstraint.h"

FVector SampleGetClosestValid(const FVector& SourcePoint, const FVector& SpacedStartDirection, float Distribution,
                              const TArray<const FPositionalConstraint*>& RelevantConstraints, float MaxSampleRange)
{
	for(int32 i = 1; i > 0; i++)
	{
		FVector Direction = SpacedStartDirection * i;
		const double DirectionLength = Direction.Length();
		if(DirectionLength > MaxSampleRange) return FVector(NAN);
		
		const double RadiusDistance = DOUBLE_PI * 2.0 * DirectionLength;
		const int32 Steps = std::round(RadiusDistance / Distribution);
		const double RotationPerStep = Distribution/RadiusDistance;

		TArray<FVector> SamplePoints;
		SamplePoints.Add(Direction);
		for(int32 j = 1; j < Steps; j++)
		{
			SamplePoints.Add(SourcePoint + Direction.RotateAngleAxisRad(RotationPerStep, FVector(0.f, 0.f, 1.f)));
		}
		FVector Result;
		if(CheckSamplesForFirstValid(Result, SamplePoints, RelevantConstraints)) return Result;
	}
	return {};
}

bool CheckSamplesForFirstValid(FVector& ValidPoint, const TArray<FVector>& SamplePoints,
                               const TArray<const FPositionalConstraint*>& RelevantConstraints)
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
		if(AreAllSatisfied)
		{
			ValidPoint = SamplePoint;
			return true;
		}
	}
	return false;
}
