// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PositionalConstraint.generated.h"


USTRUCT()
struct MAPROJECT_API FPositionalConstraint
{
	GENERATED_BODY()
public:
	virtual ~FPositionalConstraint() = default;

	bool IsConstraintSatisfied(const FVector& Position, bool RequireNavData = false, const uint8 RequiredMatchLevel = 1) const
	{ return RequiredMatchLevel <= GetMatchLevel(Position, RequireNavData); }

	virtual uint8 GetMaxMatchLevel() const { return 0; }	
	virtual uint8 GetMatchLevel(const FVector& Position, bool RequireNavData = false) const { unimplemented(); return 0; };

#if WITH_EDITORONLY_DATA
	virtual void DrawConstraintDebug(UWorld* World, FLinearColor DebugColor, float ShowTime) const { unimplemented(); }
#endif
};




/* Limitation: Collision with obstacles
 */
USTRUCT()
struct MAPROJECT_API FObstacleSpaceConstraint : public FPositionalConstraint
{
	GENERATED_BODY();
public:
	int32 MatchLevelFactor;
	UPROPERTY()
	UShapeComponent* RequiredSpace;
	UPROPERTY()
	TArray<AActor*> IrrelevantObstacles;
	
	FObstacleSpaceConstraint();
	FObstacleSpaceConstraint(UShapeComponent* NewRequiredSpace, const TArray<AActor*>& NewIrrelevantObstacles,
		int32 NewMatchLevelFactor = 4);
	
	virtual uint8 GetMaxMatchLevel() const override { return MatchLevelFactor; }	
	virtual uint8 GetMatchLevel(const FVector& Position, bool RequireNavData = false) const override;
};



/* Parent class for all player relative limitations 
 */
USTRUCT()
struct MAPROJECT_API FPlayerRelativeConstraint : public FPositionalConstraint
{
	GENERATED_BODY()
public:
	UPROPERTY()
	AController* AnchorController;

	FPlayerRelativeConstraint(): AnchorController(nullptr){}

	FPlayerRelativeConstraint(AController* Anchor): AnchorController(Anchor){}
};


/* Limitation: Distance from the player (with support minimal & optimal requirements)
 */
USTRUCT()
struct MAPROJECT_API FPlayerDistanceConstraint : public FPlayerRelativeConstraint
{
	GENERATED_BODY();
public:	
	FPlayerDistanceConstraint();
	FPlayerDistanceConstraint(AController* Anchor);
	
	virtual uint8 GetMaxMatchLevel() const override { return 2; }	
	virtual uint8 GetMatchLevel(const FVector& Position, bool RequireNavData = false) const override;
	
protected:
	virtual bool SatisfiesOptimal(const FVector& Position, bool RequireNavData) const{ checkNoEntry(); return false; }
	virtual bool SatisfiesMinimal(const FVector& Position, bool RequireNavData) const{ checkNoEntry(); return false; }
};

/* Limitation: Inside a circular ring around the player (with support for minimal & optimal requirements)
 */
USTRUCT()
struct MAPROJECT_API FCircularDistanceConstraint : public FPlayerDistanceConstraint
{
	GENERATED_BODY();
public:
	UPROPERTY(EditAnywhere)
	float MaxRadius;
	UPROPERTY(EditAnywhere)
	float MinRadius;
	
	UPROPERTY(EditAnywhere, AdvancedDisplay)
	float OptimalMaxRadius;
	UPROPERTY(EditAnywhere, AdvancedDisplay)
	float OptimalMinRadius;

	

	FCircularDistanceConstraint();
	FCircularDistanceConstraint(AController* Anchor);

#if WITH_EDITORONLY_DATA
	virtual void DrawConstraintDebug(UWorld* World, FLinearColor DebugColor, float ShowTime) const override;
	void DrawOldConstraintDebug(UWorld* World, const FVector& Position, FLinearColor DebugColor, float ShowTime) const;
#endif
	
protected:
	virtual bool SatisfiesOptimal(const FVector& Position, bool RequireNavData) const override;
	virtual bool SatisfiesMinimal(const FVector& Position, bool RequireNavData) const override;
};

enum class EWorldConstraintZone : uint8
{
	Invalid,
	Northeast,
	Northwest,
	Southwest,
	Southeast
};

inline EWorldConstraintZone operator-(EWorldConstraintZone ConstraintZone)
{
	switch (ConstraintZone)
	{
	case EWorldConstraintZone::Northeast: return EWorldConstraintZone::Southwest;
	case EWorldConstraintZone::Northwest: return EWorldConstraintZone::Southeast;
	case EWorldConstraintZone::Southwest: return EWorldConstraintZone::Northeast;
	case EWorldConstraintZone::Southeast: return EWorldConstraintZone::Northwest;
	default: return EWorldConstraintZone::Invalid;
	}
}

/* The constraint imposed by dividing the world into zones
 * that are relative to the player
 */
USTRUCT()
struct MAPROJECT_API FPlayerRelativeWorldZoneConstraint : public FPlayerRelativeConstraint
{
	GENERATED_BODY();
public:

	EWorldConstraintZone ConstraintZone;

	FPlayerRelativeWorldZoneConstraint() : ConstraintZone(EWorldConstraintZone::Invalid)	{}

	FPlayerRelativeWorldZoneConstraint(AController* SourcePlayer) : FPlayerRelativeConstraint(SourcePlayer),
	ConstraintZone(EWorldConstraintZone::Invalid){}

	FPlayerRelativeWorldZoneConstraint(AController* SourcePlayer, FVector TargetPosition);

	virtual uint8 GetMaxMatchLevel() const override { return 2; }
	virtual uint8 GetMatchLevel(const FVector& Position, bool RequireNavData = false) const override;

#if WITH_EDITORONLY_DATA
	virtual void DrawConstraintDebug(UWorld* World, FLinearColor DebugColor, float ShowTime) const override;
	void DrawOldConstraintDebug(UWorld* World, const FVector& Position, FLinearColor DebugColor, float ShowTime) const;
	static void DrawOldConstraintDebugStatic(UWorld* World, const FVector& Position, EWorldConstraintZone CurrentZone,
		FLinearColor DebugColor, float ShowTime);
#endif
	
protected:
	EWorldConstraintZone CalculateTargetZone(FVector TargetPosition) const;
};



namespace CustomHelperFunctions
{
	static bool ShapeTraceMultiByProfile(UWorld* WorldContext, UShapeComponent* ShapeComponent, FName ProfileName,
	const TArray<AActor*>& ActorsToIgnore, TArray<FHitResult>& HitResults);

	static bool ShapeTraceMultiByProfile(UWorld* WorldContext, UShapeComponent* ShapeComponent, FVector Location,
		FName ProfileName, const TArray<AActor*>& ActorsToIgnore, TArray<FHitResult>& HitResults);

	/**
	 * @brief Gets the closest valid point by sampling the environment in circles
	 * @param ResultingLocation Is set to the closest valid point if there is one inside MaxSampleRange
	 * @param SourcePoint The point around which the valid locations are sampled
	 * @param SpacedStartDirection The direction where the first sample of each circle will be taken and the spacing between every circle
	 * @param Distribution How far the sample points are from each other
	 * @param RelevantConstraints The constraints that have to be met for a point to be considered valid
	 * @param MaxSampleRange The maximal range in which samples will be taken
	 * @param ProjectionHalfHeight the height the samples can be distant from the navmesh to stay valid
	 * @param World The world context object
	 * @param DebuggingEnabled whether debugging elements should be drawn
	 * @return whether a valid point was found*/
	bool SampleGetClosestValid(FVector& ResultingLocation, const FVector& SourcePoint, const FVector& SpacedStartDirection,
	                           float Distribution, const TArray<const FPositionalConstraint*>& RelevantConstraints, float MaxSampleRange,
	                           float ProjectionHalfHeight, UWorld* World, bool RequireNavData, bool DebuggingEnabled = false);

	bool CheckSamplesForFirstValid(FVector& ValidPoint, const TArray<FVector>& SamplePoints,
	                               const TArray<const FPositionalConstraint*>& RelevantConstraints, uint32 TotalMaxMatch, bool
	                               RequireNavData, UWorld* World = nullptr, bool DebuggingEnabled = false);
}
