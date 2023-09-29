// Fill out your copyright notice in the Description page of Project Settings.


#include "PositionalConstraint.h"

#include "NavigationSystem.h"
#include "Components/BoxComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"


FObstacleSpaceConstraint::FObstacleSpaceConstraint() : MatchLevelFactor(4), RequiredSpace(nullptr)
{
}

FObstacleSpaceConstraint::FObstacleSpaceConstraint(UShapeComponent* NewRequiredSpace,
    const FVector& NewOffsetFromCenter, const FVector& NewFacingToPoint, const TArray<AActor*>& NewIrrelevantObstacles,
    int32 NewMatchLevelFactor) :
		MatchLevelFactor(NewMatchLevelFactor), OffsetFromCenter(NewOffsetFromCenter), FacingToPoint(NewFacingToPoint),
		RequiredSpace(NewRequiredSpace), IrrelevantObstacles(NewIrrelevantObstacles)
{
}


uint8 FObstacleSpaceConstraint::GetMatchLevel(const FVector& Position, UNavigationSystemV1* NavigationSystem) const
{
	check(IsValid(RequiredSpace) && IsValid(RequiredSpace->GetWorld()));

	const FVector ResultingPosition = Position + (FacingToPoint - Position).Rotation().RotateVector(OffsetFromCenter);
	
	TArray<FHitResult> HitResults;
	UConstraintsFunctionLibrary::ShapeTraceMultiForObjects(RequiredSpace->GetWorld(), RequiredSpace, ResultingPosition,
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
		//TODO: this leads to inconsistency
		NavigationSystem->GetPathLength(AnchorController->GetWorld(), Position,
			AnchorController->GetPawn()->GetActorLocation(), PathLength);

		//only use path length (which seems to be an approximation
		//(as discussed here: https://forums.unrealengine.com/t/get-path-length-inconsistent-results/285948/7)
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



bool UConstraintsFunctionLibrary::ShapeTraceMultiForObjects(UWorld* WorldContext, UShapeComponent* ShapeComponent,
	const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes, const TArray<AActor*>& ActorsToIgnore,
	TArray<FHitResult>& HitResults)
{
	return ShapeTraceMultiForObjects(WorldContext, ShapeComponent, ShapeComponent->GetComponentLocation(), ObjectTypes,
	                                 ActorsToIgnore, HitResults);
}

bool UConstraintsFunctionLibrary::ShapeTraceMultiForObjects(UWorld* WorldContext, UShapeComponent* ShapeComponent, FVector Location,
                               const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
                               const TArray<AActor*>& ActorsToIgnore, TArray<FHitResult>& HitResults)
{
	if(ShapeComponent->IsA(USphereComponent::StaticClass()))
	{
		const USphereComponent* SphereComponent = CastChecked<USphereComponent>(ShapeComponent);
		return UKismetSystemLibrary::SphereTraceMultiForObjects(WorldContext, Location,
			Location, SphereComponent->GetScaledSphereRadius(), ObjectTypes,
			true, ActorsToIgnore, EDrawDebugTrace::None, HitResults, true);
	}
	if(ShapeComponent->IsA(UBoxComponent::StaticClass()))
	{
		const UBoxComponent* BoxComponent = CastChecked<UBoxComponent>(ShapeComponent);
		return UKismetSystemLibrary::BoxTraceMultiForObjects(WorldContext, Location,
			Location, BoxComponent->GetScaledBoxExtent(),
			BoxComponent->GetComponentRotation(), ObjectTypes,
			true, ActorsToIgnore, EDrawDebugTrace::None, HitResults, true);
	}
	unimplemented();
	return false;
}

bool UConstraintsFunctionLibrary::SampleGetClosestValid(FVector& ResultingLocation, const FVector& SourcePoint, const FVector& SpacedStartDirection,
                           float Distribution, float MaxSampleRange, const TArray<const FPositionalConstraint*>& RelevantConstraints,
                           UWorld* World, float ProjectionHalfHeight, ETestType TestType, bool ForceNoNavPath, bool DebuggingEnabled)
{
	uint32 MaxPossibleMatch = 0;
	for(const FPositionalConstraint* Constraint : RelevantConstraints)
	{
		MaxPossibleMatch += Constraint->GetMaxMatchLevel();
	}

	TArray<FVector> AcceptableLocations;
	for(uint32 i = 1; true; i++)
	{
		FVector Direction = SpacedStartDirection * i;
		const double DirectionLength = Direction.Length();
		if(DirectionLength > MaxSampleRange) break;
	
		const int32 Steps = round(DOUBLE_PI * 2.0 * DirectionLength / Distribution);
		const double RotationPerStep = Distribution/DirectionLength;

		TArray<FVector> SamplePoints;
		for(int32 j = 0; j < Steps; j++)
		{
			SamplePoints.Add(SourcePoint +
				Direction.RotateAngleAxisRad(RotationPerStep * static_cast<double>(j),
					FVector(0.f, 0.f, 1.f)));
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
										  const TArray<const FPositionalConstraint*>& RelevantConstraints, uint32 TotalMaxMatch,
										  UWorld* World, const FVector& ProjectionExtent, ETestType TestType,
										  bool ForceNoNavPath, bool DebuggingEnabled)
{
	ValidPoint = FVector(NAN);
	TTuple<uint32, FVector> CurrentBest;
	CurrentBest.Key = 0;
	for(const FVector& SamplePoint : SamplePoints)
	{
		const uint32 TotalMatch = GetMatchLevel(SamplePoint, RelevantConstraints, World,
			ProjectionExtent,  TestType, ForceNoNavPath);
		const bool IsMaximalValueReached = TotalMatch >= TotalMaxMatch;
#if WITH_EDITORONLY_DATA
		if(DebuggingEnabled)
		{
			if(IsMaximalValueReached) DrawDebugPoint(World, SamplePoint + FVector(0.f, 0.f, 20.f),
				10.f, FColor(0, 255, 0), false, 1, SDPG_World);
			else
			{
				const float ReachFactor = static_cast<float>(TotalMatch)/static_cast<float>(TotalMaxMatch);
					DrawDebugPoint(World, SamplePoint + FVector(0.f, 0.f, 20.f), 10.f,
					FColor(0, ReachFactor*255.f, (1.f - ReachFactor)*255.f),
					false, 0.1, SDPG_World);
			}
		}
#endif
	
		if(IsMaximalValueReached)
		{
			ValidPoint = SamplePoint;
			return true;
		}
		if(CurrentBest.Key < TotalMatch)
		{
			CurrentBest.Key = TotalMatch;
			CurrentBest.Value = SamplePoint;
		}
	}
	//if there is a valid but not optimal point, false is returned but ValidPoint is still set
	if(CurrentBest.Key > 0) ValidPoint = CurrentBest.Value;
	return false;
}

uint32 UConstraintsFunctionLibrary::GetMatchLevel(const FVector& TestLocation, const TArray<const FPositionalConstraint*>& RelevantConstraints,
                     UWorld* World, const FVector& DistanceFromNavMesh, ETestType TestType, bool ForceNoNavPath)
{
	UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetNavigationSystem(World);
		
	if(!DistanceFromNavMesh.ContainsNaN()){
		FNavLocation ProjectedLocation;
		if(!NavigationSystem->ProjectPointToNavigation(TestLocation ,ProjectedLocation, DistanceFromNavMesh)) return 0;
	}
		
	uint32 TotalMatch = 0;
	for(const FPositionalConstraint* Constraint : RelevantConstraints)
	{
		const uint32 Match = Constraint->GetMatchLevel(TestLocation, ForceNoNavPath ? nullptr : NavigationSystem);
		if(TestType == RequireAllValid && Match == 0)
		{
			return 0;
		}
		if(TestType == RequireAllOptimal && Match != Constraint->GetMaxMatchLevel()) return 0;
		TotalMatch += Match;
	}
	return TotalMatch;
}
