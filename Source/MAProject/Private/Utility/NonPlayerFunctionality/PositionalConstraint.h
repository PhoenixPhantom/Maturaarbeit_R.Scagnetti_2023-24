// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PositionalConstraint.generated.h"


USTRUCT()
struct MAPROJECT_API FPositionalConstraint
{
	GENERATED_BODY()
public:
	FPositionalConstraint() = default;
	virtual ~FPositionalConstraint() = default;
	
	virtual bool IsConstraintSatisfied(FVector Position) const
	{ unimplemented(); return false; };
};

/**
 * @brief Gets the closest valid point by sampling the environment in circles
 * @param SourcePoint The point around which the valid locations are sampled
 * @param SpacedStartDirection The direction where the first sample of each circle will be taken and the spacing between every circle
 * @param Distribution How far the sample points are from each other
 * @param RelevantConstraints The constraints that have to be met for a point to be considered valid
 * @return the closest valid point if there is one that can be found in max_val(int32) test cycles*/
static FVector SampleGetClosestValid(const FVector& SourcePoint, const FVector& SpacedStartDirection, float Distribution,
                                     const TArray<FPositionalConstraint>& RelevantConstraints);

static bool CheckSamplesForFirstValid(FVector& ValidPoint, const TArray<FVector>& SamplePoints,
	const TArray<FPositionalConstraint>& RelevantConstraints);