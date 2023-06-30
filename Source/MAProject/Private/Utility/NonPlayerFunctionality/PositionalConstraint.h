// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PositionalConstraint.generated.h"


USTRUCT()
struct MAPROJECT_API FPositionalConstraint
{
	GENERATED_BODY()
public:
	UPROPERTY()
	AActor* Owner;
	
	FPositionalConstraint() : Owner(nullptr){}
	FPositionalConstraint(AActor* SourceOwner) : Owner(SourceOwner){}
	virtual ~FPositionalConstraint(){};
	
	virtual bool IsConstraintSatisfied(FVector Position) const
	{ unimplemented(); return false; };

#if WITH_EDITORONLY_DATA
	virtual void DrawConstraintDebug(UWorld* World, FLinearColor DebugColor, float ShowTime) const { unimplemented(); };
#endif
};

static bool ShapeTraceMultiByProfile(UWorld* WorldContext, UShapeComponent* ShapeComponent, FName ProfileName,
	TArray<AActor*> ActorsToIgnore, TArray<FHitResult>& HitResults);

/**
 * @brief Gets the closest valid point by sampling the environment in circles
 * @param ResultingLocation Is set to the closest valid point if there is one inside MaxSampleRange
 * @param RequiredSpace The space that is needed at the minimum around a valid point
 * @param SourcePoint The point around which the valid locations are sampled
 * @param SpacedStartDirection The direction where the first sample of each circle will be taken and the spacing between every circle
 * @param Distribution How far the sample points are from each other
 * @param RelevantConstraints The constraints that have to be met for a point to be considered valid
 * @param MaxSampleRange The maximal range in which samples will be taken
 * @param ProjectionHalfHeight the height the samples can be distant from the navmesh to stay valid
 * @param World The world context object
 * @param DebuggingEnabled whether debugging elements should be drawn
 * @return whether a valid point was found*/
bool SampleGetClosestValid(FVector& ResultingLocation, UShapeComponent* RequiredSpace, const FVector& SourcePoint,
	const FVector& SpacedStartDirection, float Distribution,
	const TArray<const FPositionalConstraint*>& RelevantConstraints, float MaxSampleRange, float ProjectionHalfHeight,
	UWorld* World, bool DebuggingEnabled = false);

bool CheckSamplesForFirstValid(FVector& ValidPoint, const TArray<FVector>& SamplePoints,
	const TArray<const FPositionalConstraint*>& RelevantConstraints, UWorld* World = nullptr, bool DebuggingEnabled = false);