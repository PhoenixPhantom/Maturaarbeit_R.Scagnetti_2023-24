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
	FRequiredSpace(): Box(nullptr), Sphere(nullptr){}
	FRequiredSpace(UBoxComponent* RequiredSpace) : Box(RequiredSpace), Sphere(nullptr){}
	FRequiredSpace(USphereComponent* RequiredSpace) : Box(nullptr), Sphere(RequiredSpace){}
	UPROPERTY(EditAnywhere)
	UBoxComponent* Box;
	UPROPERTY(EditAnywhere)
	USphereComponent* Sphere;

	float GetMinimalRadius() const;
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

	inline static uint32 ConstraintID = 0;
	virtual uint32 GetConstraintType() const { return ConstraintID; }
	virtual bool IsOfType(uint32 ID) const { return ConstraintID == ID; }
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

	inline static uint32 ConstraintID = 1;
	virtual uint32 GetConstraintType() const override { return ConstraintID; }
	virtual bool IsOfType(uint32 ID) const override { return ConstraintID == ID || Super::IsOfType(ID); }
	
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

	inline static uint32 ConstraintID = 2;
	virtual uint32 GetConstraintType() const override { return ConstraintID; }
	virtual bool IsOfType(uint32 ID) const override { return ConstraintID == ID || Super::IsOfType(ID); }
	
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

	inline static uint32 ConstraintID = 3;
	virtual uint32 GetConstraintType() const override { return ConstraintID; }
	virtual bool IsOfType(uint32 ID) const override { return ConstraintID == ID || Super::IsOfType(ID); }
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

	inline static uint32 ConstraintID = 4;
	virtual uint32 GetConstraintType() const override { return ConstraintID; }
	virtual bool IsOfType(uint32 ID) const override { return ConstraintID == ID || Super::IsOfType(ID); }

	virtual uint8 GetMatchLevel(const FVector& Position, UNavigationSystemV1* NavigationSystem) const override;

#if WITH_EDITORONLY_DATA
	virtual void DrawConstraintDebug(UWorld* World, FLinearColor DebugColor, float ShowTime) const override;
	void DrawOldConstraintDebug(const UWorld* World, const FVector& Position, FLinearColor DebugColor, float ShowTime) const;
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

	inline static uint32 ConstraintID = 5;
	virtual uint32 GetConstraintType() const override { return ConstraintID; }
	virtual bool IsOfType(uint32 ID) const override { return ConstraintID == ID || Super::IsOfType(ID); }

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


USTRUCT()
struct MAPROJECT_API FPointGenerator
{
	GENERATED_BODY();
public:
	virtual ~FPointGenerator() = default;
	virtual FVector GetSamplePoint(uint64 Index) const{ return FVector(NAN); }
	virtual uint64 GetIndexRange() const{ return 0; }
};


USTRUCT(meta=(DeprecatedStruct, DeprecationMessage="May contain inefficient or outright incorrect calculations. Only used to prove it's inefficiency."))
struct MAPROJECT_API FInefficientPointGenerator : public FPointGenerator
{
	GENERATED_BODY();
public:
	FVector SourcePoint;
	FVector SpacedStartDirection;
	float Distribution;
	uint64 NumOfCircles;
	void SetProperties(const FVector& NewSourcePoint, const FVector& NewSpacedStartDirection,
	                   float NewDistribution, float NewMaxSampleRange);
	virtual FVector GetSamplePoint(uint64 Index) const override;
	virtual uint64 GetIndexRange() const override;
};


//The goto point generator used to directly limit the searched points when given a FCircularDistanceConstraint
USTRUCT()
struct MAPROJECT_API FCircularPointsGenerator : public FPointGenerator
{
	GENERATED_BODY();
public:
	FVector SourcePoint;
	FVector StartDirection;
	float MinimalLength;
	float Density;
	uint64 NumOfCircles;

	FCircularPointsGenerator();
	FCircularPointsGenerator(const FCircularDistanceConstraint& SourceConstraint, const FVector& NewStartDirection,
		float NewDensity){ SetProperties(SourceConstraint, NewStartDirection, NewDensity); }
	void SetProperties(const FCircularDistanceConstraint& SourceConstraint, const FVector& NewStartDirection,
		float NewDensity);
	virtual FVector GetSamplePoint(uint64 Index) const override;
	virtual uint64 GetIndexRange() const override;
};

USTRUCT()
struct MAPROJECT_API FSquarePointGenerator : public FPointGenerator
{
	GENERATED_BODY()
public:
	FVector SourcePoint;
	FVector MaxShiftSpace;
	FVector StartDirection;
	FVector SideDirection;
	float Distribution;
	uint64 NumOfRows;
	uint64 PointsPerRow;
	void SetProperties(const FVector& NewSourcePoint, const FVector& NewStartDirection,
	                   float NewDistribution, float MaxLength, float MaxWidth);
	virtual FVector GetSamplePoint(uint64 Index) const override;
	virtual uint64 GetIndexRange() const override;
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
	/// @param ResultingLocation Is set to the closest valid point if there is one inside MaxSampleRange
	/// @param PointGenerator Object generating the sample points according to the given rules
	/// @param RelevantConstraints The constraints that have to be met for a point to be considered valid
	/// @param World The world context object
	/// @param ProjectionExtent the amount by which a sample position may be shifted (+ and -) so it is on the navigation mesh
	/// @param InstantSuccessCondition The type of test result that we try to find. Immediately ends the calculation when found.
	/// @param InstantFailureCondition The type of test result that is required of a point to even be considered. Immediately discards the tested point when not met.
	/// @param ForceNoNavPath if distance calculations are forced to rely on air distance (even if nav distance is demanded)
	/// @param DebuggingEnabled whether debugging elements should be drawn
	/// @return whether a valid point was found
	static bool GetBestPositionSampled(FVector& ResultingLocation, const FPointGenerator& PointGenerator,
		const TArray<const FPositionalConstraint*>& RelevantConstraints, UWorld* World, const FVector& ProjectionExtent,
		ETestType InstantSuccessCondition = RequireOneValid, ETestType InstantFailureCondition = RequireAllValid,
		bool ForceNoNavPath = false, bool DebuggingEnabled = false);
	
	/// 
	/// @param TestLocation The location to get the match level for
	/// @param RelevantConstraints The conditions based on which the match level will be generated
	/// @param World The world context object
	/// @param DistanceFromNavMesh the distance from the Navmesh that the point is required (NAN means that no maximal distance is required) 
	/// @param InstantFailureCondition the Requirements that have to be fulfilled for the test to return a result > 0
	/// @param ForceNoNavPath if distance calculations are forced to rely on air distance (even if nav distance is demanded)
	/// @return The match level of the test location
	static uint32 GetMatchLevel(const FVector& TestLocation, const TArray<const FPositionalConstraint*>& RelevantConstraints,
		UWorld* World, const FVector& DistanceFromNavMesh = FVector(NAN),
		ETestType InstantFailureCondition = RequireAllValid, bool ForceNoNavPath = false, bool DebuggingEnabled = false);

#if WITH_EDITORONLY_DATA
	static void DebugConstraint(const FVector& TestLocation, const FPositionalConstraint* Constraint, FColor Mask, UWorld* WorldContext);
#endif
};
