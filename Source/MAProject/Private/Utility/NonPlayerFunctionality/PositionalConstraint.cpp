// Fill out your copyright notice in the Description page of Project Settings.


#include "PositionalConstraint.h"

#include "NavigationSystem.h"
#include "Components/BoxComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"


FPlayerDistanceConstraint::FPlayerDistanceConstraint()
{
	
}

FPlayerDistanceConstraint::FPlayerDistanceConstraint(AController* Anchor) : FPositionalConstraint(Anchor)
{
	
}

uint8 FPlayerDistanceConstraint::GetMatchLevel(const FVector& Position, bool RequireNavData) const
{
	return SatisfiesOptimal(Position, RequireNavData) ? 2 : SatisfiesMinimal(Position, RequireNavData) ? 1 : 0;
}


FCircularDistanceConstraint::FCircularDistanceConstraint(): MaxRadius(300.f), MinRadius(200.f), OptimalMaxRadius(0),
                                                            OptimalMinRadius(0)
{
}

FCircularDistanceConstraint::FCircularDistanceConstraint(AController* Anchor): FPlayerDistanceConstraint(Anchor),
																			MaxRadius(300.f), MinRadius(200.f),
																			OptimalMaxRadius(0), OptimalMinRadius(0)
{
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

bool FCircularDistanceConstraint::SatisfiesOptimal(const FVector& Position, bool RequireNavData) const
{
	double Distance;
	if(!RequireNavData) Distance = FVector::Distance(Position, AnchorController->GetPawn()->GetActorLocation());
	else
	{
		UNavigationSystemV1::GetPathLength(AnchorController->GetWorld(), Position,
			AnchorController->GetPawn()->GetActorLocation(), Distance);
	}
	return Distance <= OptimalMaxRadius && Distance >= OptimalMinRadius;
}

bool FCircularDistanceConstraint::SatisfiesMinimal(const FVector& Position, bool RequireNavData) const
{
	double Distance;
	//TODO: getting the distance in this context doesn't seem to yield the correct calculation results
	if(!RequireNavData) Distance = (Position - AnchorController->GetPawn()->GetActorLocation()).Length();
	else
	{
		UNavigationSystemV1::GetPathLength(AnchorController->GetWorld(), Position,
			AnchorController->GetPawn()->GetActorLocation(), Distance);
	}
	GLog->Log("|" + Position.ToString() + " - " + AnchorController->GetPawn()->GetActorLocation().ToString() +
		"| = " + FString::SanitizeFloat(Distance) + " when calculated for movement");
	return Distance <= MaxRadius && Distance >= MinRadius;
}



uint8 FPlayerRelativeWorldZoneConstraint::GetMatchLevel(const FVector& Position, bool RequireNavData) const
{
	if(ConstraintZone == EWorldConstraintZone::Invalid) return 0;
	const EWorldConstraintZone TargetZone = CalculateTargetZone(Position);
	if(TargetZone == ConstraintZone) return 2;
	if(TargetZone != -ConstraintZone) return 1;
	return 0;
}

#if WITH_EDITORONLY_DATA
FPlayerRelativeWorldZoneConstraint::FPlayerRelativeWorldZoneConstraint(AController* SourcePlayer,
	FVector TargetPosition): FPositionalConstraint(SourcePlayer)
{
	ConstraintZone = CalculateTargetZone(TargetPosition);
}

EWorldConstraintZone FPlayerRelativeWorldZoneConstraint::CalculateTargetZone(
	FVector TargetPosition) const
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




namespace CustomHelperFunctions
{
	bool ShapeTraceMultiByProfile(UWorld* WorldContext, UShapeComponent* ShapeComponent, FName ProfileName,
		const TArray<AActor*>& ActorsToIgnore, TArray<FHitResult>& HitResults)
	{
		return ShapeTraceMultiByProfile(WorldContext, ShapeComponent, ShapeComponent->GetComponentLocation(), ProfileName,
			ActorsToIgnore, HitResults);
	}

	bool ShapeTraceMultiByProfile(UWorld* WorldContext, UShapeComponent* ShapeComponent, FVector Location,
		FName ProfileName, const TArray<AActor*>& ActorsToIgnore, TArray<FHitResult>& HitResults)
	{
		if(ShapeComponent->IsA(USphereComponent::StaticClass()))
		{
			const USphereComponent* SphereComponent = CastChecked<USphereComponent>(ShapeComponent);
			return UKismetSystemLibrary::SphereTraceMultiByProfile(WorldContext, Location,
				Location, SphereComponent->GetScaledSphereRadius(), ProfileName,
				true, ActorsToIgnore, EDrawDebugTrace::None, HitResults, true);
		}
		if(ShapeComponent->IsA(UBoxComponent::StaticClass()))
		{
			const UBoxComponent* BoxComponent = CastChecked<UBoxComponent>(ShapeComponent);
			return UKismetSystemLibrary::BoxTraceMultiByProfile(WorldContext, Location,
				Location, BoxComponent->GetScaledBoxExtent(),
				BoxComponent->GetComponentRotation(), ProfileName,
				true, ActorsToIgnore, EDrawDebugTrace::None, HitResults, true);
		}
		unimplemented();
		return false;
	}

	bool SampleGetClosestValid(FVector& ResultingLocation, UShapeComponent* RequiredSpace, AActor* Querier,
	                           const TArray<AActor*>& IrrelevantObstacles, const FVector& SourcePoint,
	                           const FVector& SpacedStartDirection, float Distribution,
	                           const TArray<const FPositionalConstraint*>& RelevantConstraints, float MaxSampleRange, float ProjectionHalfHeight,
	                           UWorld* World,
	                           bool RequireNavData, bool DebuggingEnabled)
	{
		uint32 MaxPossibleMatch = 0;
		for(const FPositionalConstraint* Constraint : RelevantConstraints)
		{
			MaxPossibleMatch += Constraint->GetMaxMatchLevel();
		}

		TArray<FVector> AcceptableLocations;
		TArray<AActor*> ActorsToIgnore = IrrelevantObstacles;
		ActorsToIgnore.Add(Querier);
		for(uint32 i = 1; true; i++)
		{
			FVector Direction = SpacedStartDirection * i;
			const double DirectionLength = Direction.Length();
			if(DirectionLength > MaxSampleRange) break;
		
			const int32 Steps = round(DOUBLE_PI * 2.0 * DirectionLength / Distribution);
			const double RotationPerStep = Distribution/DirectionLength;

			FVector ProjectionExtent(Distribution/2.f, Distribution/2.f, ProjectionHalfHeight);
			TArray<FVector> SamplePoints;
			UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetNavigationSystem(World);
			for(int32 j = 0; j < Steps; j++)
			{
				FNavLocation ProjectedLocation;
				const FVector TestLocation = SourcePoint +
					Direction.RotateAngleAxisRad(RotationPerStep * static_cast<double>(j),
						FVector(0.f, 0.f, 1.f));
				if(!NavigationSystem->ProjectPointToNavigation(TestLocation ,ProjectedLocation, ProjectionExtent))
				{
					if(DebuggingEnabled) DrawDebugPoint(World, TestLocation + FVector(0.f, 0.f, 20.f),
						10.f, FColor(255, 0, 0),false, 1, SDPG_World);
					continue;
				}
				TArray<FHitResult> HitResults;
				ShapeTraceMultiByProfile(World, RequiredSpace, ProjectedLocation, "PawnScanner",
					ActorsToIgnore, HitResults);
				if(HitResults.IsEmpty()) SamplePoints.Add(ProjectedLocation);
			}
			if(CheckSamplesForFirstValid(ResultingLocation, SamplePoints, RelevantConstraints, MaxPossibleMatch,
			                             RequireNavData, World, DebuggingEnabled)) return true;
			if(!ResultingLocation.ContainsNaN()) AcceptableLocations.Add(ResultingLocation);
		}

		//Determine the best (if any) acceptable location
		if(CheckSamplesForFirstValid(ResultingLocation, AcceptableLocations, RelevantConstraints,
		                             MaxPossibleMatch - 1, RequireNavData, World, DebuggingEnabled) ||
		                             !ResultingLocation.ContainsNaN()) return true;
		return false;
	}

	bool CheckSamplesForFirstValid(FVector& ValidPoint, const TArray<FVector>& SamplePoints,
	                               const TArray<const FPositionalConstraint*>& RelevantConstraints, uint32 TotalMaxMatch,
	                               bool RequireNavData, UWorld* World, bool DebuggingEnabled)
	{
		ValidPoint = FVector(NAN);
		TTuple<uint32, FVector> CurrentBest;
		CurrentBest.Key = 0;
		for(const FVector& SamplePoint : SamplePoints)
		{
			uint32 TotalMatch = 0;
			for(const FPositionalConstraint* Constraint : RelevantConstraints)
			{
				TotalMatch += Constraint->GetMatchLevel(SamplePoint);
			}
			const bool IsMaximalValueReached = TotalMatch >= TotalMaxMatch;
#if WITH_EDITORONLY_DATA
			if(DebuggingEnabled)
			{
				if(IsMaximalValueReached) DrawDebugPoint(World, SamplePoint + FVector(0.f, 0.f, 20.f), 10.f, FColor(0, 255, 0),
					false, 1, SDPG_World);
				else DrawDebugPoint(World, SamplePoint + FVector(0.f, 0.f, 20.f), 10.f, FColor(0, 0, 255),
					false, 0.1, SDPG_World);
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
}
