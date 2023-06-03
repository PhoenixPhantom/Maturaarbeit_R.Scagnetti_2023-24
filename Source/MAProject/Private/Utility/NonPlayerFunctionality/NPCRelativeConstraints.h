// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PositionalConstraint.h"
#include "NPCRelativeConstraints.generated.h"

/* For all constraints that are imposed by NPCs
 */
USTRUCT()
struct MAPROJECT_API FNpcRelativeConstraint : public FPositionalConstraint
{
	GENERATED_BODY();
public:
	UPROPERTY()
	AActor* Npc;
	
	UPROPERTY(EditAnywhere)
	FVector PositionOffset;	
	
	FNpcRelativeConstraint() : FPositionalConstraint(), Npc(nullptr){}
	FNpcRelativeConstraint(AActor* SourceNpc) : FPositionalConstraint(), Npc(SourceNpc){}
};

/* The constraint that is imposed by
 * NPCs not engaged in combat
 */
USTRUCT()
struct MAPROJECT_API FNonCombatConstraint : public FNpcRelativeConstraint
{
	GENERATED_BODY();
public:
	UPROPERTY(EditAnywhere)
	float Radius;

	FNonCombatConstraint() : FNpcRelativeConstraint(), Radius(10.f){}
	FNonCombatConstraint(AActor* SourceNpc) : FNpcRelativeConstraint(SourceNpc), Radius(10.f){}
	
	virtual bool IsConstraintSatisfied(FVector Position) const override
	{ return FVector::Distance(Npc->GetActorLocation() + PositionOffset, Position) > Radius; }
};

/* The constraint that is imposed on
 * active combat participants
 */
USTRUCT()
struct MAPROJECT_API FActiveCombatConstraint : public FNpcRelativeConstraint
{
	GENERATED_BODY();
public:
	UPROPERTY(EditAnywhere)
	float Radius;

	FActiveCombatConstraint() : FNpcRelativeConstraint(), Radius(10.f){}
	FActiveCombatConstraint(AActor* SourceNpc) : FNpcRelativeConstraint(SourceNpc), Radius(10.f){}

	virtual bool IsConstraintSatisfied(FVector Position) const override
	{ return FVector::Distance(Npc->GetActorLocation() + PositionOffset, Position) > Radius; }
};

/* The constraint that is imposed by
 * passive combat participants
 * (shaped like a bent rectangle)
 */
USTRUCT()
struct MAPROJECT_API FPassiveCombatConstraint : public FNpcRelativeConstraint
{
	GENERATED_BODY();
public:
	UPROPERTY()
	AActor* OrientationCenter;
	
	UPROPERTY(EditAnywhere)
	float VerticalSize;
	UPROPERTY(EditAnywhere)
	float HorizontalSize;

	FPassiveCombatConstraint();
	FPassiveCombatConstraint(AActor* SourceNpc, AActor* SourceOrientationCenter);

	virtual bool IsConstraintSatisfied(FVector Position) const override;
};
