// Fill out your copyright notice in the Description page of Project Settings.


#include "PositionalConstraint.h"

#include "NavigationSystem.h"
#include "Components/BoxComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"


float FRequiredSpace::GetMinimalRadius() const
{
	if(IsValid(Sphere)) return Sphere->GetScaledSphereRadius();
	if(IsValid(Box)) return Box->GetScaledBoxExtent().GetAbsMin();
	return 0.f;
}

UShapeComponent* FRequiredSpace::GetShape() const
{
	if(Box == nullptr) return Sphere;
	return Box;
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
	if(RequiredSpace.Sphere != nullptr)
	{
		const FVector LocationOffset = (FacingToPoint - Position).Rotation().
			RotateVector(RequiredSpace.Sphere->GetRelativeLocation());
		if(FVector::Distance(Position, ReserverLocation + LocationOffset) >=
			FMath::Max(RequiredSpace.Sphere->GetScaledSphereRadius(), OtherRadius))
		{
			return MatchLevelFactor;
		}
		return 0;
	}
	if(RequiredSpace.Box != nullptr)
	{
		const FRotator ToWorldSpaceRotation = (FacingToPoint - Position).Rotation();
		const FVector BoxLocation =
			ToWorldSpaceRotation.RotateVector(RequiredSpace.Box->GetRelativeLocation()) + ReserverLocation;
		const FVector BoxExtent = ToWorldSpaceRotation.RotateVector(RequiredSpace.Box->GetScaledBoxExtent());

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
	double Distance = FVector::Distance(Position, AnchorController->GetPawn()->GetActorLocation());
	if(bUseNavPath && IsValid(NavigationSystem))
	{
		double PathLength;
		if(NavigationSystem->GetPathLength(AnchorController->GetWorld(), Position,
			AnchorController->GetPawn()->GetNavAgentLocation(), PathLength)
			!= ENavigationQueryResult::Success) return 0;

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

void FCircularDistanceConstraint::DrawOldConstraintDebug(const UWorld* World, const FVector& Position, FLinearColor DebugColor,
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

void FInefficientPointGenerator::SetProperties(const FVector& NewSourcePoint, const FVector& NewSpacedStartDirection,
                                               float NewDistribution, float NewMaxSampleRange)
{
	NumOfCircles = floor(NewMaxSampleRange / NewSpacedStartDirection.Length());
	
	//experimentally, it can be seen, that the points created by this algorithms are distributed too
	//grid like in the direction of the sampling
	//(the best distribution seems to be around 135° (clockwise) from the sampling direction)
	//RotateAngleAxis seems to work with a counterclockwise input
	SpacedStartDirection = NewSpacedStartDirection.RotateAngleAxis(135.0, FVector(0.0, 0.0, 1.0));
	SourcePoint = NewSourcePoint;
	Distribution = NewDistribution;
}

FVector FInefficientPointGenerator::GetSamplePoint(uint64 Index) const
{
	uint64 CurrentCircleIndex = 0;
	const float PointsOnCircleMultiplier = DOUBLE_PI * 2.0 * SpacedStartDirection.Length() / Distribution;
	uint64 IndicesRemaining = Index;
	uint64 PointsOnCurrentCircle = 0;
	for(uint64 i = 0; i < Index; i++)
	{
		PointsOnCurrentCircle = ceil(PointsOnCircleMultiplier * i);
		if(IndicesRemaining > PointsOnCurrentCircle)
		{
			IndicesRemaining -= PointsOnCurrentCircle;
			continue;
		}
		CurrentCircleIndex = i;
		break;
	}

	const FVector Direction = SpacedStartDirection * static_cast<double>(CurrentCircleIndex);
	const double DirectionLength = Direction.Length();
	
	//360°/Radius * Distribution = 360°/PointsOnCircle = RotationPerStep <==>
	//2*PI/(2*PI*DirLen / Dist) = Dist/DirLen = RotationPerStep 
	const double RotationPerStep = Distribution/DirectionLength;
	return  SourcePoint + Direction.RotateAngleAxisRad(RotationPerStep * static_cast<double>(IndicesRemaining),
			FVector(0.f, 0.f, 1.f));
}

uint64 FInefficientPointGenerator::GetIndexRange() const
{
	return DOUBLE_TWO_PI * SpacedStartDirection.Length() / Distribution * static_cast<float>(NumOfCircles) * static_cast<float>(NumOfCircles);
}

FCircularPointsGenerator::FCircularPointsGenerator(): MinimalLength(0), Density(0), NumOfCircles(0)
{
}

void FCircularPointsGenerator::SetProperties(const FCircularDistanceConstraint& SourceConstraint,
                                            const FVector& NewStartDirection, float NewDensity)
{
	SourcePoint = SourceConstraint.AnchorController->GetPawn()->GetActorLocation();
	StartDirection = NewStartDirection.GetSafeNormal();
	MinimalLength = SourceConstraint.MinRadius;
	Density = NewDensity;
	NumOfCircles = ceil((SourceConstraint.MaxRadius - SourceConstraint.MinRadius) * NewDensity);
}

FVector FCircularPointsGenerator::GetSamplePoint(uint64 Index) const
{
	uint64 CurrentCircleIndex = 0;
	uint64 IndicesRemaining = Index;
	uint64 PointsOnCurrentCircle = 0;
	for(uint64 i = 0; i < Index; i++)
	{
		PointsOnCurrentCircle = ceil(Density * (DOUBLE_TWO_PI * (MinimalLength + static_cast<double>(i) / Density)));
		if(IndicesRemaining >= PointsOnCurrentCircle)
		{
			IndicesRemaining -= PointsOnCurrentCircle;
			continue;
		}
		CurrentCircleIndex = i;
		break;
	}

	const double Radius = MinimalLength + static_cast<double>(CurrentCircleIndex) / Density;
	const double RadiansPerStep = PointsOnCurrentCircle == 0 ? 0.0 : DOUBLE_TWO_PI/static_cast<double>(PointsOnCurrentCircle);
	const double RotationAmount = RadiansPerStep * ceil(static_cast<double>(IndicesRemaining)/2.0) *
		(IndicesRemaining & 0b1 ? -1.0 : 1.0);
	return  SourcePoint + (StartDirection * Radius).RotateAngleAxisRad(RotationAmount,FVector(0.f, 0.f, 1.f));
}

uint64 FCircularPointsGenerator::GetIndexRange() const
{
	//Density * (DOUBLE_TWO_PI * (MinimalLength + CircleIndex/Density)) = PointsOnCircle[Index]
	return static_cast<float>(NumOfCircles) * (DOUBLE_TWO_PI * (Density * MinimalLength + 0.5*static_cast<float>(NumOfCircles)));
}

void FSquarePointGenerator::SetProperties(const FVector& NewSourcePoint, const FVector& NewStartDirection,
                                          float NewDistribution, float MaxLength, float MaxWidth)
{
	NumOfRows = ceil(MaxLength / NewDistribution);
	PointsPerRow = ceil(MaxWidth / NewDistribution);
	
	StartDirection = NewStartDirection.GetSafeNormal();
	SideDirection = StartDirection.Cross(FVector(0.0, 0.0, 1.0)).GetSafeNormal();
	SourcePoint = NewSourcePoint - SideDirection * (static_cast<double>(PointsPerRow - 1)/2.0 * NewDistribution);
	Distribution = NewDistribution;
}

FVector FSquarePointGenerator::GetSamplePoint(uint64 Index) const
{
	const double RowIndex = floor(Index / PointsPerRow);
	const double PointIndex = floor(Index % PointsPerRow);
	return  SourcePoint + StartDirection * (RowIndex * Distribution) + SideDirection * (PointIndex * Distribution);
}

uint64 FSquarePointGenerator::GetIndexRange() const
{
	return NumOfRows * PointsPerRow;
}

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
	if(RequiredSpace.Sphere != nullptr)
	{
		return UKismetSystemLibrary::SphereTraceMultiForObjects(WorldContext, Location,
			Location, RequiredSpace.Sphere->GetScaledSphereRadius(), ObjectTypes,
			true, ActorsToIgnore, EDrawDebugTrace::None, HitResults, true);
	}
	if(RequiredSpace.Box != nullptr)
	{
		//TODO: using component rotation makes no sense here
		return UKismetSystemLibrary::BoxTraceMultiForObjects(WorldContext, Location,
			Location, RequiredSpace.Box->GetScaledBoxExtent(),
			RequiredSpace.Box->GetComponentRotation(), ObjectTypes,
			true, ActorsToIgnore, EDrawDebugTrace::None, HitResults, true);
	}
	unimplemented();
	return false;
}

bool UConstraintsFunctionLibrary::GetBestPositionSampled(FVector& ResultingLocation,
	const FPointGenerator& PointGenerator, const TArray<const FPositionalConstraint*>& RelevantConstraints,
	UWorld* World, const FVector& ProjectionExtent, ETestType InstantSuccessCondition, ETestType InstantFailureCondition,
	bool ForceNoNavPath, bool DebuggingEnabled)
{
	uint32 MaxPossibleMatch = 0;
	for(const FPositionalConstraint* Constraint : RelevantConstraints)
	{
		MaxPossibleMatch += Constraint->GetMaxMatchLevel();
	}

	//this case would need a lot of additional handling, so we don't support it
	if(InstantSuccessCondition == RequireAllValid && InstantFailureCondition != RequireAllValid)
	{
		checkNoEntry();
		return false;
	}
	
	TTuple<uint64, FVector> CurrentBest;
	CurrentBest.Key = 0;
	CurrentBest.Value = FVector(NAN);
	for(uint64 i = 0; i < PointGenerator.GetIndexRange(); i++)
	{
		FVector SamplePoint = PointGenerator.GetSamplePoint(i);
		const uint32 Match = GetMatchLevel(SamplePoint, RelevantConstraints, World,ProjectionExtent,
			InstantFailureCondition, ForceNoNavPath, DebuggingEnabled);
		if((InstantSuccessCondition == RequireAllOptimal && Match >= MaxPossibleMatch) ||
			((InstantSuccessCondition == RequireAllValid || InstantSuccessCondition == RequireOneValid) && Match != 0))
		{
			ResultingLocation = SamplePoint;
			return true;
		}
		if(InstantFailureCondition == RequireAllOptimal && Match < MaxPossibleMatch) continue;
		if(CurrentBest.Key < Match)
		{
			CurrentBest.Key = Match;
			CurrentBest.Value = SamplePoint;
		}
	}
	ResultingLocation = CurrentBest.Value;
	if(ResultingLocation.ContainsNaN()) return false;
	return true;
}

uint32 UConstraintsFunctionLibrary::GetMatchLevel(const FVector& TestLocation,
	const TArray<const FPositionalConstraint*>& RelevantConstraints, UWorld* World, const FVector& DistanceFromNavMesh,
	ETestType InstantFailureCondition, bool ForceNoNavPath, bool DebuggingEnabled)
{
	if(TestLocation.ContainsNaN()) return 0;
	UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetNavigationSystem(World);


	FVector LocationToTest = TestLocation;
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
		LocationToTest = ProjectedLocation;
	}
		
	uint32 TotalMatch = 0;
	for(const FPositionalConstraint* Constraint : RelevantConstraints)
	{
		const uint32 Match = Constraint->GetMatchLevel(LocationToTest, ForceNoNavPath ? nullptr : NavigationSystem);
		if(InstantFailureCondition == RequireAllValid && Match == 0)
		{
#if WITH_EDITORONLY_DATA
			if(DebuggingEnabled) DebugConstraint(LocationToTest, Constraint, FColor(1, 1, 1), World);
#endif
			return 0;
		}
		if(InstantFailureCondition == RequireAllOptimal && Match != Constraint->GetMaxMatchLevel())
		{
#if WITH_EDITORONLY_DATA
				if(DebuggingEnabled) DebugConstraint(LocationToTest, Constraint, FColor(50, 50, 50), World);
#endif
				return 0;
		}
		TotalMatch += Match;
	}
	
#if WITH_EDITORONLY_DATA
	if(DebuggingEnabled) DrawDebugPoint(World, LocationToTest,10.f, FColor(255, 255, 255) , false, 1, SDPG_World);
#endif
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
