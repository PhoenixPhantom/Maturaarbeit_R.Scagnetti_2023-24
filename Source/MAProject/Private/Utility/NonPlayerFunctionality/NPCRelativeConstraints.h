// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PositionalConstraint.h"
#include "NPCRelativeConstraints.generated.h"

void DrawDebugCircularFrustum(UWorld* World, const FVector& Center, const FVector& Direction, float Radius1, float Radius2,
	int32 NumSegments,
	FLinearColor DebugColor, float ShowTime);

/* For all constraints that are imposed by NPCs
 */
USTRUCT()
struct MAPROJECT_API FNpcRelativeConstraints : public FPositionalConstraint
{
	GENERATED_BODY();
public:
	
	UPROPERTY(EditAnywhere)
	FVector PositionOffset;	
	
	FNpcRelativeConstraints() : FPositionalConstraint(), PositionOffset(0.f){}
	FNpcRelativeConstraints(AActor* SourceNpc) : FPositionalConstraint(SourceNpc), PositionOffset(0.f){}
};

/* The constraint that is imposed by
 * NPCs not engaged in combat
 */
USTRUCT()
struct MAPROJECT_API FNonCombatConstraint : public FNpcRelativeConstraints
{
	GENERATED_BODY();
public:
	UPROPERTY(EditAnywhere)
	float Radius;

	FNonCombatConstraint() : FNpcRelativeConstraints(), Radius(10.f){}
	FNonCombatConstraint(AActor* SourceNpc) : FNpcRelativeConstraints(SourceNpc), Radius(10.f){}
	
	virtual bool IsConstraintSatisfied(FVector Position) const override
	{ return FVector::Distance(Owner->GetActorLocation() + PositionOffset, Position) > Radius; }

#if WITH_EDITORONLY_DATA
	virtual void DrawConstraintDebug(UWorld* World, FLinearColor DebugColor, float ShowTime) const override;
#endif
};

/* The constraint that is imposed on
 * active combat participants
 */
USTRUCT()
struct MAPROJECT_API FActiveCombatConstraint : public FNpcRelativeConstraints
{
	GENERATED_BODY();
public:
	UPROPERTY(EditAnywhere)
	float Radius;

	FActiveCombatConstraint() : FNpcRelativeConstraints(), Radius(10.f){}
	FActiveCombatConstraint(AActor* SourceNpc) : FNpcRelativeConstraints(SourceNpc), Radius(10.f){}

	virtual bool IsConstraintSatisfied(FVector Position) const override
	{ return FVector::Distance(Owner->GetActorLocation() + PositionOffset, Position) > Radius; }

#if WITH_EDITORONLY_DATA
	virtual void DrawConstraintDebug(UWorld* World, FLinearColor DebugColor, float ShowTime) const override;
#endif
};

/* The constraint that is imposed by
 * passive combat participants
 * (shaped like a bent rectangle)
 */
USTRUCT()
struct MAPROJECT_API FPassiveCombatConstraint : public FNpcRelativeConstraints
{
	GENERATED_BODY();
public:
	UPROPERTY()
	AController* OrientationCenter;
	
	UPROPERTY(EditAnywhere)
	float VerticalSize;
	UPROPERTY(EditAnywhere)
	float HorizontalSize;

	FPassiveCombatConstraint();
	FPassiveCombatConstraint(AActor* SourceNpc, AController* SourceOrientationCenter);

	virtual bool IsConstraintSatisfied(FVector Position) const override;

#if WITH_EDITORONLY_DATA
	virtual void DrawConstraintDebug(UWorld* World, FLinearColor DebugColor, float ShowTime) const override;
#endif
};
