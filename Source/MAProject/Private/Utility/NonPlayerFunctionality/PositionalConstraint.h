// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PositionalConstraint.generated.h"


class UBoxComponent;
class USphereComponent;
class UNavigationSystemV1;

USTRUCT()
struct MAPROJECT_API FRequiredSpace
{
	GENERATED_BODY()
public:
	FRequiredSpace(): RequiredSpaceBox(nullptr), RequiredSpaceSphere(nullptr){}
	FRequiredSpace(UBoxComponent* RequiredSpace) : RequiredSpaceBox(RequiredSpace), RequiredSpaceSphere(nullptr){}
	FRequiredSpace(USphereComponent* RequiredSpace) : RequiredSpaceBox(nullptr), RequiredSpaceSphere(RequiredSpace){}
	UPROPERTY(EditAnywhere)
	UBoxComponent* RequiredSpaceBox;
	UPROPERTY(EditAnywhere)
	USphereComponent* RequiredSpaceSphere;

	UShapeComponent* GetShape() const;
};

USTRUCT()
struct MAPROJECT_API FPositionalConstraint
{
	GENERATED_BODY()
public:
	bool bUseNavPath;

	FPositionalConstraint(bool UseNavPath = false) : bUseNavPath(UseNavPath){}
	virtual ~FPositionalConstraint() = default;

	bool IsConstraintSatisfied(const FVector& Position, UNavigationSystemV1* NavigationSystem = nullptr, const uint8 RequiredMatchLevel = 1) const
	{ return RequiredMatchLevel <= GetMatchLevel(Position, NavigationSystem); }	

	virtual uint8 GetMaxMatchLevel() const { return 0; }
	virtual uint8 GetMatchLevel(const FVector& Position,  UNavigationSystemV1* NavigationSystem) const{ unimplemented(); return 0;};

#if WITH_EDITORONLY_DATA
	virtual void DrawConstraintDebug(UWorld* World, FLinearColor DebugColor, float ShowTime) const { unimplemented(); }
#endif
};

/* Limitation: Reserved space by others
 */
USTRUCT()
struct MAPROJECT_API FReservedSpaceConstraint : public FPositionalConstraint
{
	GENERATED_BODY();
public:
	int32 MatchLevelFactor;
	FVector ReserverLocation;
	FVector FacingToPoint;
	float OtherRadius;

	FRequiredSpace RequiredSpace;
	
	FReservedSpaceConstraint();
	FReservedSpaceConstraint(const FRequiredSpace& NewRequiredSpace, const FVector& NewReserverLocation,
	                         const FVector& NewFacingToPoint, float CheckerRadius = 0.f, int32 NewMatchLevelFactor = 1);
	
	virtual uint8 GetMaxMatchLevel() const override { return MatchLevelFactor; }
	virtual uint8 GetMatchLevel(const FVector& Position, UNavigationSystemV1* NavigationSystem) const override;
};


/* Limitation: Collision with obstacles
 */
USTRUCT()
struct MAPROJECT_API FObstacleSpaceConstraint : public FPositionalConstraint
{
	GENERATED_BODY();
public:
	int32 MatchLevelFactor;
	FVector FacingToPoint;
	FRequiredSpace RequiredSpace;
	UPROPERTY()
	TArray<AActor*> IrrelevantObstacles;
	
	FObstacleSpaceConstraint();
	FObstacleSpaceConstraint(const FRequiredSpace& NewRequiredSpace, const FVector& NewFacingToPoint,
	                         const TArray<AActor*>& NewIrrelevantObstacles, int32 NewMatchLevelFactor = 1);
	
	virtual uint8 GetMaxMatchLevel() const override { return MatchLevelFactor; }
	virtual uint8 GetMatchLevel(const FVector& Position, UNavigationSystemV1* NavigationSystem) const override;
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

	FPlayerRelativeConstraint(bool UseNavPath = false): FPositionalConstraint(UseNavPath), AnchorController(nullptr){}

	FPlayerRelativeConstraint(AController* Anchor, bool UseNavPath = false): FPositionalConstraint(UseNavPath),
		AnchorController(Anchor){}
};

/* Limitation: Inside a circular ring around the player (with support for minimal & optimal requirements)
 */
USTRUCT()
struct MAPROJECT_API FCircularDistanceConstraint : public FPlayerRelativeConstraint
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

	

	FCircularDistanceConstraint(bool UseNavPath = false);
	FCircularDistanceConstraint(AController* Anchor, bool UseNavPath = false);

	virtual uint8 GetMatchLevel(const FVector& Position, UNavigationSystemV1* NavigationSystem) const override;

#if WITH_EDITORONLY_DATA
	virtual void DrawConstraintDebug(UWorld* World, FLinearColor DebugColor, float ShowTime) const override;
	void DrawOldConstraintDebug(UWorld* World, const FVector& Position, FLinearColor DebugColor, float ShowTime) const;
#endif
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

	FPlayerRelativeWorldZoneConstraint(AController* SourcePlayer, const FVector& TargetPosition);

	virtual uint8 GetMaxMatchLevel() const override { return 2; }
	virtual uint8 GetMatchLevel(const FVector& Position, UNavigationSystemV1* NavigationSystem) const override;

#if WITH_EDITORONLY_DATA
	virtual void DrawConstraintDebug(UWorld* World, FLinearColor DebugColor, float ShowTime) const override;
	void DrawOldConstraintDebug(UWorld* World, const FVector& Position, FLinearColor DebugColor, float ShowTime) const;
	static void DrawOldConstraintDebugStatic(UWorld* World, const FVector& Position, EWorldConstraintZone CurrentZone,
		FLinearColor DebugColor, float ShowTime);
#endif
	
protected:
	EWorldConstraintZone CalculateTargetZone(const FVector& TargetPosition) const;
};

UCLASS(meta=(ScriptName="ConstraintsLibrary"))
class UConstraintsFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	static bool ShapeTraceMultiForObjects(UWorld* WorldContext, const FRequiredSpace& RequiredSpace,
										  const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
										  const TArray<AActor*>& ActorsToIgnore, TArray<FHitResult>& HitResults);

	static bool ShapeTraceMultiForObjects(UWorld* WorldContext, const FRequiredSpace& RequiredSpace, FVector Location,
	                                      const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectType,
	                                      const TArray<AActor*>& ActorsToIgnore, TArray<FHitResult>& HitResults);

	enum ETestType : uint8
	{
		RequireOneValid,
		RequireAllValid,
		RequireAllOptimal
	};
	
	/// @brief Gets the closest valid point by sampling the environment in circles
	/// @param ResultingLocation 
	/// @param ResultingLocation Is set to the closest valid point if there is one inside MaxSampleRange
	/// @param SourcePoint The point around which the valid locations are sampled
	/// @param SpacedStartDirection The direction where the first sample of each circle will be taken and the spacing between every circle
	/// @param Distribution How far the sample points are from each other
	/// @param RelevantConstraints The constraints that have to be met for a point to be considered valid
	/// @param MaxSampleRange The maximal range in which samples will be taken
	/// @param World The world context object
	/// @param ProjectionHalfHeight the height the samples can be distant from the navmesh to stay valid
	/// @param TestType the Requirements that have to be fulfilled for an individual test to be considered a valid option
	/// @param ForceNoNavPath if distance calculations are forced to rely on air distance (even if nav distance is demanded)
	/// @param DebuggingEnabled whether debugging elements should be drawn
	/// @return whether a valid point was found
	static bool SampleGetClosestValid(FVector& ResultingLocation, const FVector& SourcePoint, const FVector& SpacedStartDirection,
	                                  float Distribution, float MaxSampleRange, const TArray<const FPositionalConstraint*>& RelevantConstraints,
	                                  UWorld* World, float ProjectionHalfHeight, ETestType TestType = RequireOneValid,
	                                  bool ForceNoNavPath = false, bool DebuggingEnabled = false);

	static bool CheckSamplesForFirstValid(FVector& ValidPoint, const TArray<FVector>& SamplePoints,
	                                      const TArray<const FPositionalConstraint*>& RelevantConstraints, uint32 TotalMaxMatch,
	                                      UWorld* World, const FVector& ProjectionExtent, ETestType TestType = RequireOneValid,
	                                      bool ForceNoNavPath = false, bool DebuggingEnabled = false);
	
	/// 
	/// @param TestLocation The location to get the match level for
	/// @param RelevantConstraints The conditions based on which the match level will be generated
	/// @param World The world context object
	/// @param DistanceFromNavMesh the distance from the Navmesh that the point is required (NAN means that no maximal distance is required) 
	/// @param TestType the Requirements that have to be fulfilled for the test to return a result > 0
	/// @param ForceNoNavPath if distance calculations are forced to rely on air distance (even if nav distance is demanded)
	/// @return The match level of the test location
	static uint32 GetMatchLevel(const FVector& TestLocation, const TArray<const FPositionalConstraint*>& RelevantConstraints,
		UWorld* World, const FVector& DistanceFromNavMesh = FVector(NAN),
		ETestType TestType = RequireOneValid, bool ForceNoNavPath = false);
};
