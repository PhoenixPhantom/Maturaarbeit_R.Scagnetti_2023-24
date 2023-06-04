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
	UPROPERTY()
	AActor* Player;
	
	FPlayerRelativeConstraint() : FPositionalConstraint(), Player(nullptr){}
	FPlayerRelativeConstraint(AActor* SourcePlayer) : FPositionalConstraint(), Player(SourcePlayer){}
};


/* The constraint imposed by the distance
 * from player in both max and min
 */
USTRUCT()
struct MAPROJECT_API FPlayerDistanceConstraint : public FPlayerRelativeConstraint
{
	GENERATED_BODY();
public:
	UPROPERTY(EditAnywhere)
	float MaxRadius;
	UPROPERTY(EditAnywhere)
	float MinRadius;

	FPlayerDistanceConstraint() : FPlayerRelativeConstraint(), MaxRadius(300.f), MinRadius(200.f){}
	FPlayerDistanceConstraint(AActor* SourcePlayer) : FPlayerRelativeConstraint(SourcePlayer),
		MaxRadius(300.f), MinRadius(200.f){}
	
	virtual bool IsConstraintSatisfied(FVector Position) const override;

#if WITH_EDITORONLY_DATA
	virtual void DrawConstraintDebug(UWorld* World, FLinearColor DebugColor, float ShowTime) const override;
	void DrawOldConstraintDebug(UWorld* World, const FVector& Position, FLinearColor DebugColor, float ShowTime) const;
#endif
};

/* The constraint imposed by dividing the world into zones
 * that are relative to the player
 */
USTRUCT()
struct MAPROJECT_API FPlayerRelativeWorldZoneConstraint : public FPlayerRelativeConstraint
{
	GENERATED_BODY();
public:
	enum EWorldConstraintZone : uint8
	{
		Invalid,
		Northeast,
		Northwest,
		Southwest,
		Southeast
	};

	EWorldConstraintZone ConstraintZone;

	FPlayerRelativeWorldZoneConstraint() : FPlayerRelativeConstraint(), ConstraintZone(Invalid)	{}

	FPlayerRelativeWorldZoneConstraint(AActor* SourcePlayer) : FPlayerRelativeConstraint(SourcePlayer),
	ConstraintZone(Invalid){}
	
	virtual bool IsConstraintSatisfied(FVector Position) const override{ return CalculateTargetZone(Position) == ConstraintZone; }
	EWorldConstraintZone CalculateTargetZone(FVector TargetPosition) const;

#if WITH_EDITORONLY_DATA
	virtual void DrawConstraintDebug(UWorld* World, FLinearColor DebugColor, float ShowTime) const override;
	void DrawOldConstraintDebug(UWorld* World, const FVector& Position, FLinearColor DebugColor, float ShowTime) const;
#endif
};