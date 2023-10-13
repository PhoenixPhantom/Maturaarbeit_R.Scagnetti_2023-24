// Fill out your copyright notice in the Description page of Project Settings.


#include "PositionalConstraint.h"

#include "NavigationSystem.h"
#include "Components/BoxComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"


UShapeComponent* FRequiredSpace::GetShape() const
{
	if(RequiredSpaceBox == nullptr) return RequiredSpaceSphere;
	return RequiredSpaceBox;
}

FReservedSpaceConstraint::FReservedSpaceConstraint(): MatchLevelFactor(1), OtherRadius(0.f)
{
}

FReservedSpaceConstraint::FReservedSpaceConstraint(const FRequiredSpace& NewRequiredSpace,
    const FVector& NewReserverLocation, const FVector& NewFacingToPoint, float CheckerRadius, int32 NewMatchLevelFactor) :
		MatchLevelFactor(NewMatchLevelFactor), ReserverLocation(NewReserverLocation), FacingToPoint(NewFacingToPoint),
		OtherRadius(CheckerRadius), RequiredSpace(NewRequiredSpace)
{
}

uint8 FReservedSpaceConstraint::GetMatchLevel(const FVector& Position, UNavigationSystemV1* NavigationSystem) const
{
	if(RequiredSpace.RequiredSpaceSphere != nullptr)
	{
		const FVector LocationOffset = (FacingToPoint - Position).Rotation().
			RotateVector(RequiredSpace.RequiredSpaceSphere->GetRelativeLocation());
		if(FVector::Distance(Position, ReserverLocation + LocationOffset) >=
			FMath::Max(RequiredSpace.RequiredSpaceSphere->GetScaledSphereRadius(), OtherRadius))
		{
			return MatchLevelFactor;
		}
		return 0;
	}
	if(RequiredSpace.RequiredSpaceBox != nullptr)
	{
		const FRotator ToWorldSpaceRotation = (FacingToPoint - Position).Rotation();
		const FVector BoxLocation =
			ToWorldSpaceRotation.RotateVector(RequiredSpace.RequiredSpaceBox->GetRelativeLocation()) + ReserverLocation;
		const FVector BoxExtent = ToWorldSpaceRotation.RotateVector(RequiredSpace.RequiredSpaceBox->GetScaledBoxExtent());

		const bool InsideX = (Position.X < BoxLocation.X + BoxExtent.X) && (Position.X > BoxLocation.X - BoxExtent.X);
		const bool InsideY = (Position.Y < BoxLocation.Y + BoxExtent.Y) && (Position.Y > BoxLocation.Y - BoxExtent.Y);
		const bool InsideZ = (Position.Z < BoxLocation.Z + BoxExtent.Z) && (Position.Z > BoxLocation.Z - BoxExtent.Z);
		if(!InsideX && !InsideY && !InsideZ)
		{
			return MatchLevelFactor;
		}
		return 0;
	}
	unimplemented();
	return 0;
}

FObstacleSpaceConstraint::FObstacleSpaceConstraint() : MatchLevelFactor(4)
{
}

FObstacleSpaceConstraint::FObstacleSpaceConstraint(const FRequiredSpace& NewRequiredSpace,
const FVector& NewFacingToPoint, const TArray<AActor*>& NewIrrelevantObstacles, int32 NewMatchLevelFactor) :
	MatchLevelFactor(NewMatchLevelFactor), FacingToPoint(NewFacingToPoint),
	RequiredSpace(NewRequiredSpace), IrrelevantObstacles(NewIrrelevantObstacles)
{
	
}


uint8 FObstacleSpaceConstraint::GetMatchLevel(const FVector& Position, UNavigationSystemV1* NavigationSystem) const
{
	check(IsValid(RequiredSpace.GetShape()) && IsValid(RequiredSpace.GetShape()->GetWorld()));

	const FVector ResultingPosition =
		Position + (FacingToPoint - Position).Rotation().RotateVector(RequiredSpace.GetShape()->GetRelativeLocation());
	
	TArray<FHitResult> HitResults;
	UConstraintsFunctionLibrary::ShapeTraceMultiForObjects(RequiredSpace.GetShape()->GetWorld(), RequiredSpace, ResultingPosition,
		{UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn)}, IrrelevantObstacles, HitResults);
	if(HitResults.IsEmpty()) return MatchLevelFactor;
	return 0;
}

FCircularDistanceConstraint::FCircularDistanceConstraint(bool UseNavPath) : MaxRadius(300.f), MinRadius(200.f),
	OptimalMaxRadius(0), OptimalMinRadius(0)
{
	bUseNavPath = UseNavPath;
}

FCircularDistanceConstraint::FCircularDistanceConstraint(AController* Anchor, bool UseNavPath):
	FPlayerRelativeConstraint(Anchor, UseNavPath), MaxRadius(300.f), MinRadius(200.f), OptimalMaxRadius(0),
	OptimalMinRadius(0)
{
}

uint8 FCircularDistanceConstraint::GetMatchLevel(const FVector& Position, UNavigationSystemV1* NavigationSystem) const
{
	const FVector& TargetLocation = AnchorController->GetPawn()->GetActorLocation();
	double Distance = FVector::Distance(Position, TargetLocation);
	if(bUseNavPath && IsValid(NavigationSystem))
	{
		double PathLength;
		NavigationSystem->GetPathLength(AnchorController->GetWorld(), Position,TargetLocation, PathLength);

		//only use path length (which seems to be an approximation
		//(as discussed here: https://forums.unrealengine.com/t/get-path-length-inconsistent-results/285948/7))
		//If the result makes sense
		if(PathLength > Distance) Distance = PathLength; 
		
	}

	if(Distance <= OptimalMaxRadius && Distance >= OptimalMinRadius) return 2;
	if(Distance <= MaxRadius && Distance >= MinRadius) return 1;
	return 0;
}

#if WITH_EDITORONLY_DATA
void FCircularDistanceConstraint::DrawConstraintDebug(UWorld* World, FLinearColor DebugColor, float ShowTime) const
{
	DrawOldConstraintDebug(World, AnchorController->GetPawn()->GetActorLocation(), DebugColor, ShowTime);
}

void FCircularDistanceConstraint::DrawOldConstraintDebug(UWorld* World, const FVector& Position, FLinearColor DebugColor,
                                                       float ShowTime) const
{
	UKismetSystemLibrary::DrawDebugSphere(World, Position, OptimalMinRadius, 50, DebugColor,ShowTime);
	UKismetSystemLibrary::DrawDebugSphere(World, Position, OptimalMaxRadius, 50, DebugColor, ShowTime);
	UKismetSystemLibrary::DrawDebugSphere(World, Position, MinRadius, 50,
		(DebugColor + FLinearColor(0.f, 0.f, 0.f))/2.f,ShowTime);
	UKismetSystemLibrary::DrawDebugSphere(World, Position, MaxRadius, 50,
		(DebugColor + FLinearColor(0.f, 0.f, 0.f))/2.f, ShowTime);	
}
#endif



uint8 FPlayerRelativeWorldZoneConstraint::GetMatchLevel(const FVector& Position, UNavigationSystemV1* NavigationSystem) const
{
	if(ConstraintZone == EWorldConstraintZone::Invalid) return 0;
	const EWorldConstraintZone TargetZone = CalculateTargetZone(Position);
	if(TargetZone == ConstraintZone) return 2;
	if(TargetZone != -ConstraintZone) return 1;
	//if(TargetZone == -ConstraintZone) return 0;
	return 0;
}

#if WITH_EDITORONLY_DATA
FPlayerRelativeWorldZoneConstraint::FPlayerRelativeWorldZoneConstraint(AController* SourcePlayer,
	const FVector& TargetPosition): FPlayerRelativeConstraint(SourcePlayer)
{
	ConstraintZone = CalculateTargetZone(TargetPosition);
}

EWorldConstraintZone FPlayerRelativeWorldZoneConstraint::CalculateTargetZone(
	const FVector& TargetPosition) const
{
	FVector2d Direction = FVector2d(TargetPosition.X, TargetPosition.Y) -
		FVector2d(AnchorController->GetPawn()->GetActorLocation().X, AnchorController->GetPawn()->GetActorLocation().Y);
	Direction.Normalize();
	EWorldConstraintZone Zone;
	
	//North
	if(Direction.Y > 0)
	{
		if(Direction.X > 0) Zone = EWorldConstraintZone::Northeast;
		else Zone = EWorldConstraintZone::Northwest;
	}
	//South
	else
	{
		if(Direction.X > 0) Zone = EWorldConstraintZone::Southeast;
		else Zone = EWorldConstraintZone::Southwest;	
	}
	return Zone;
}

void FPlayerRelativeWorldZoneConstraint::DrawConstraintDebug(UWorld* World, FLinearColor DebugColor, float ShowTime) const
{
	DrawOldConstraintDebug(World, AnchorController->GetPawn()->GetActorLocation(), DebugColor, ShowTime);
}

void FPlayerRelativeWorldZoneConstraint::DrawOldConstraintDebug(UWorld* World, const FVector& Position,
	FLinearColor DebugColor, float ShowTime) const
{
	DrawOldConstraintDebugStatic(World, Position, ConstraintZone, DebugColor, ShowTime);
	DrawOldConstraintDebugStatic(World, Position, -ConstraintZone,
		FLinearColor(1.f, 1.f, 1.f, 2.f) - DebugColor, ShowTime);
}

void FPlayerRelativeWorldZoneConstraint::DrawOldConstraintDebugStatic(UWorld* World, const FVector& Position,
	EWorldConstraintZone CurrentZone, FLinearColor DebugColor, float ShowTime)
{
	const FVector Extent = FVector(1000.f, 1000.f, 50.f);
	FVector Offset = Extent;
	switch(CurrentZone)
	{
	case EWorldConstraintZone::Northwest:
		{
			Offset.X = -Offset.X;
		}
	case EWorldConstraintZone::Northeast: break;
	case EWorldConstraintZone::Southwest:
		{
			Offset.X = -Offset.X;
		}
	case EWorldConstraintZone::Southeast:
		{
			Offset.Y = -Offset.Y;
			break;
		}
	default: checkNoEntry();
	}
	
	UKismetSystemLibrary::DrawDebugBox(World, Position + Offset,
		Extent, DebugColor, FRotator::ZeroRotator, ShowTime);
}

#endif



bool UConstraintsFunctionLibrary::ShapeTraceMultiForObjects(UWorld* WorldContext, const FRequiredSpace& RequiredSpace,
	const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes, const TArray<AActor*>& ActorsToIgnore,
	TArray<FHitResult>& HitResults)
{
	return ShapeTraceMultiForObjects(WorldContext, RequiredSpace, RequiredSpace.GetShape()->GetComponentLocation(), ObjectTypes,
	                                 ActorsToIgnore, HitResults);
}

bool UConstraintsFunctionLibrary::ShapeTraceMultiForObjects(UWorld* WorldContext, const FRequiredSpace& RequiredSpace, FVector Location,
                               const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
                               const TArray<AActor*>& ActorsToIgnore, TArray<FHitResult>& HitResults)
{
	if(RequiredSpace.RequiredSpaceSphere != nullptr)
	{
		return UKismetSystemLibrary::SphereTraceMultiForObjects(WorldContext, Location,
			Location, RequiredSpace.RequiredSpaceSphere->GetScaledSphereRadius(), ObjectTypes,
			true, ActorsToIgnore, EDrawDebugTrace::None, HitResults, true);
	}
	if(RequiredSpace.RequiredSpaceBox != nullptr)
	{
		//TODO: using component rotation makes no sense here
		return UKismetSystemLibrary::BoxTraceMultiForObjects(WorldContext, Location,
			Location, RequiredSpace.RequiredSpaceBox->GetScaledBoxExtent(),
			RequiredSpace.RequiredSpaceBox->GetComponentRotation(), ObjectTypes,
			true, ActorsToIgnore, EDrawDebugTrace::None, HitResults, true);
	}
	unimplemented();
	return false;
}

bool UConstraintsFunctionLibrary::SampleGetClosestValid(FVector& ResultingLocation, const FVector& SourcePoint, const FVector& SpacedStartDirection,
                                                        double Distribution, float MaxSampleRange, const TArray<const FPositionalConstraint*>& RelevantConstraints,
                                                        UWorld* World, float ProjectionHalfHeight, ETestType TestType, bool ForceNoNavPath, bool DebuggingEnabled)
{
	uint32 MaxPossibleMatch = 0;
	for(const FPositionalConstraint* Constraint : RelevantConstraints)
	{
		MaxPossibleMatch += Constraint->GetMaxMatchLevel();
	}

	//experimentally, it can be seen, that the points created by this algorithms are distributed too
	//grid like in the direction of the sampling
	//(the best distribution seems to be around 135° (clockwise) from the sampling direction)
	//RotateAngleAxis seems to work with a counterclockwise input
	const FVector CorrectedStartDirection = SpacedStartDirection.RotateAngleAxis(135.0, FVector(0.0, 0.0, 1.0));
	TArray<FVector> AcceptableLocations;
	for(uint32 i = 1; true; i++)
	{
		FVector Direction = CorrectedStartDirection * static_cast<double>(i);
		const double DirectionLength = Direction.Length();
		if(DirectionLength > MaxSampleRange) break;
	
		const int32 PointsOnCircle = ceil(DOUBLE_PI * 2.0 * DirectionLength / Distribution);
		//360°/Radius * Distribution = 360°/PointsOnCircle = RotationPerStep <==>
		//2*PI/(2*PI*DirLen / Dist) = Dist/DirLen = RotationPerStep 
		const double RotationPerStep = Distribution/DirectionLength;

		TArray<FVector> SamplePoints;
		SamplePoints.SetNumUninitialized(PointsOnCircle);
		for(int32 j = 0; j < PointsOnCircle; j++)
		{
			SamplePoints[j] = SourcePoint + Direction.RotateAngleAxisRad(RotationPerStep * static_cast<double>(j),
				FVector(0.f, 0.f, 1.f));
		}
		if(CheckSamplesForFirstValid(ResultingLocation, SamplePoints, RelevantConstraints, MaxPossibleMatch,
				World,FVector(Distribution/2.f, Distribution/2.f, ProjectionHalfHeight),
		        TestType, ForceNoNavPath, DebuggingEnabled)) return true;
		if(!ResultingLocation.ContainsNaN()) AcceptableLocations.Add(ResultingLocation);
	}

	//Determine the best (if any) acceptable location
	CheckSamplesForFirstValid(ResultingLocation, AcceptableLocations, RelevantConstraints,
              MaxPossibleMatch - 1, World,
              FVector(Distribution/2.f, Distribution/2.f, ProjectionHalfHeight),
              TestType, ForceNoNavPath, DebuggingEnabled);
	if(!ResultingLocation.ContainsNaN()) return true;
	return false;
}

bool UConstraintsFunctionLibrary::CheckSamplesForFirstValid(FVector& ValidPoint, const TArray<FVector>& SamplePoints,
										  const TArray<const FPositionalConstraint*>& RelevantConstraints, uint32 MaxMatch,
										  UWorld* World, const FVector& ProjectionExtent, ETestType TestType,
										  bool ForceNoNavPath, bool DebuggingEnabled)
{
	ValidPoint = FVector(NAN);
	TTuple<uint32, FVector> CurrentBest;
	CurrentBest.Key = 0;
	for(const FVector& SamplePoint : SamplePoints)
	{
		const uint32 Match = GetMatchLevel(SamplePoint, RelevantConstraints, World,
		                ProjectionExtent,  TestType, ForceNoNavPath, DebuggingEnabled);

		if((TestType == ETestType::RequireAllOptimal && Match >= MaxMatch) ||
			((TestType == ETestType::RequireAllValid || TestType == ETestType::RequireOneValid) && Match != 0))
		{
			ValidPoint = SamplePoint;
			return true;
		}
		if(CurrentBest.Key < Match)
		{
			CurrentBest.Key = Match;
			CurrentBest.Value = SamplePoint;
		}
	}
	//if there is a valid but not optimal point, false is returned but ValidPoint is still set
	if(CurrentBest.Key > 0) ValidPoint = CurrentBest.Value;
	return false;
}

uint32 UConstraintsFunctionLibrary::GetMatchLevel(const FVector& TestLocation, const TArray<const FPositionalConstraint*>& RelevantConstraints,
                                                  UWorld* World, const FVector& DistanceFromNavMesh, ETestType TestType, bool ForceNoNavPath, bool DebuggingEnabled)
{
	UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetNavigationSystem(World);
		
	if(!DistanceFromNavMesh.ContainsNaN()){
		FNavLocation ProjectedLocation;
		if(!NavigationSystem->ProjectPointToNavigation(TestLocation ,ProjectedLocation, DistanceFromNavMesh))
		{
#if WITH_EDITORONLY_DATA
			if(DebuggingEnabled) DrawDebugPoint(World, TestLocation + FVector(0.f, 0.f, 0.f),
		10.f, FColor(0, 0, 0) , false, 1, SDPG_World);
#endif
			return 0;
		}
	}
		
	uint32 TotalMatch = 0;
	for(const FPositionalConstraint* Constraint : RelevantConstraints)
	{
		const uint32 Match = Constraint->GetMatchLevel(TestLocation, ForceNoNavPath ? nullptr : NavigationSystem);
		if(TestType == RequireAllValid && Match == 0)
		{
#if WITH_EDITORONLY_DATA
			if(DebuggingEnabled) DebugConstraint(TestLocation, Constraint, FColor(1, 1, 1), World);
#endif
			return 0;
		}
		if(TestType == RequireAllOptimal && Match != Constraint->GetMaxMatchLevel())
		{
#if WITH_EDITORONLY_DATA
			if(DebuggingEnabled) DebugConstraint(TestLocation, Constraint, FColor(50, 50, 50), World);
#endif
			return 0;
		}
		TotalMatch += Match;
	}
	return TotalMatch;
}

#if WITH_EDITORONLY_DATA
void UConstraintsFunctionLibrary::DebugConstraint(const FVector& TestLocation, const FPositionalConstraint* Constraint,
	FColor Mask, UWorld* WorldContext)
{
	FColor Color;
	if(Constraint->IsOfType(FCircularDistanceConstraint::ConstraintID))
	{
		Color = FColor(0, 255, 0);
	}
	else if(Constraint->IsOfType(FPlayerRelativeWorldZoneConstraint::ConstraintID))
	{
		Color = FColor(255, 0, 255);
	}
	else if(Constraint->IsOfType(FObstacleSpaceConstraint::ConstraintID))
	{
		Color = FColor(255, 0, 0);
	}
	else if(Constraint->IsOfType(FReservedSpaceConstraint::ConstraintID))
	{
		Color = FColor(0, 0, 255);
	}
	DrawDebugPoint(WorldContext, TestLocation + FVector(0.f, 0.f, 0.f),
		10.f, FColor(FMath::Min(Color.R * Mask.R + Mask.R, 255),
			FMath::Min(Color.G * Mask.G + Mask.G, 255),
			FMath::Min(Color.B * Mask.B + Mask.B, 255)) , false, 1, SDPG_World);
}
#endif
