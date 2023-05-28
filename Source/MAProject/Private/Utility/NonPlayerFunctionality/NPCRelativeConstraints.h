// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PositionalConstraint.h"
#include "NPCRelativeConstraints.generated.h"

/* For all constraints that are imposed by NPCs
 */
USTRUCT()
struct MAPROJECT_API FNPCRelativeConstraint : public FPositionalConstraint
{
	GENERATED_BODY();
public:
	virtual bool IsConstraintSatisfied(FVector Position) override;
};

/* The constraint that is imposed by
 * NPCs not engaged in combat
 */
USTRUCT()
struct MAPROJECT_API FNonCombatConstraint : public FNPCRelativeConstraint
{
	GENERATED_BODY();
public:
	virtual bool IsConstraintSatisfied(FVector Position) override;
};

/* The constraint that is imposed on
 * active combat participants
 */
USTRUCT()
struct MAPROJECT_API FActiveCombatConstraint : public FNPCRelativeConstraint
{
	GENERATED_BODY();
public:
	virtual bool IsConstraintSatisfied(FVector Position) override;
};

/* The constraint that is imposed by
 * passive combat participants
 */
USTRUCT()
struct MAPROJECT_API FPassiveCombatConstraint : public FNPCRelativeConstraint
{
	GENERATED_BODY();
public:
	virtual bool IsConstraintSatisfied(FVector Position) override;
};
