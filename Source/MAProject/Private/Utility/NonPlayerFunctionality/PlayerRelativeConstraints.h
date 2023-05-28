// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PositionalConstraint.h"
#include "PlayerRelativeConstraints.generated.h"


/* For all constraints that are imposed by the player
 */
USTRUCT()
struct MAPROJECT_API FPlayerRelativeConstraint : public FPositionalConstraint
{
	GENERATED_BODY();
public:
	virtual bool IsConstraintSatisfied(FVector Position) override;
};


/* The constraint imposed by the distance
 * from player in both max and min
 */
USTRUCT()
struct MAPROJECT_API FPlayerDistanceConstraint : public FPlayerRelativeConstraint
{
	GENERATED_BODY();
public:
	virtual bool IsConstraintSatisfied(FVector Position) override;
};

/* The constraint imposed by dividing the world into zones
 * that are relative to the player
 */
USTRUCT()
struct MAPROJECT_API FPlayerRelativeWorldZoneConstraint : public FPlayerRelativeConstraint
{
	GENERATED_BODY();
public:
	virtual bool IsConstraintSatisfied(FVector Position) override;
};