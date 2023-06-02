// Fill out your copyright notice in the Description page of Project Settings.


#include "PositionalConstraint.h"

FVector SampleGetClosestValid(const FVector& SourcePoint, const FVector& SpacedStartDirection, float Distribution,
	const TArray<FPositionalConstraint>& RelevantConstraints)
{
	const float ValidDistribution = std::round(Distribution);
	for(int32 i = 1; i > 0; i++)
	{
		FVector Direction = SpacedStartDirection * i;
		TArray<FVector> SamplePoints;
		SamplePoints.Add(Direction);
		const float RotationPerTurn = PI * 2.f * Direction.Length() / ValidDistribution;
		for(int32 j = 1; j < ValidDistribution ; j++)
		{
			SamplePoints.Add(Direction.RotateAngleAxisRad(RotationPerTurn, FVector(0.f, 0.f, 1.f)) + SourcePoint);
		}
		FVector Result;
		if(CheckSamplesForFirstValid(Result, SamplePoints, RelevantConstraints)) return Result;
	}
	checkNoEntry();
	return FVector(NAN);
}

bool CheckSamplesForFirstValid(FVector& ValidPoint, const TArray<FVector>& SamplePoints,
	const TArray<FPositionalConstraint>& RelevantConstraints)
{
	for(const FVector& SamplePoint : SamplePoints)
	{
		bool AreAllSatisfied = true;
		for(const FPositionalConstraint& Constraint : RelevantConstraints)
		{
			if(!Constraint.IsConstraintSatisfied(SamplePoint))
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
