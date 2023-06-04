// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerRelativeConstraints.h"

#include "Kismet/KismetSystemLibrary.h"

bool FPlayerDistanceConstraint::IsConstraintSatisfied(FVector Position) const
{
	const float Distance = FVector::Distance(Position, Player->GetActorLocation());
	return Distance <= MaxRadius && Distance >= MinRadius;
}

#if WITH_EDITORONLY_DATA
void FPlayerDistanceConstraint::DrawConstraintDebug(UWorld* World, FLinearColor DebugColor, float ShowTime) const
{
	DrawOldConstraintDebug(World, Player->GetActorLocation(), DebugColor, ShowTime);
}

void FPlayerDistanceConstraint::DrawOldConstraintDebug(UWorld* World, const FVector& Position, FLinearColor DebugColor,
                                                       float ShowTime) const
{
	UKismetSystemLibrary::DrawDebugSphere(World, Position, MinRadius, 50, DebugColor,ShowTime);
	UKismetSystemLibrary::DrawDebugSphere(World, Position, MaxRadius, 50, DebugColor, ShowTime);
}
#endif

FPlayerRelativeWorldZoneConstraint::EWorldConstraintZone FPlayerRelativeWorldZoneConstraint::CalculateTargetZone(
	FVector TargetPosition) const
{
	FVector2d Direction = FVector2d(TargetPosition.X, TargetPosition.Y) - FVector2d(Player->GetActorLocation().X, Player->GetActorLocation().Y);
	Direction.Normalize();
	EWorldConstraintZone Zone;
	
	//North
	if(Direction.Y > 0)
	{
		if(Direction.X > 0) Zone = Northeast;
		else Zone = Northwest;
	}
	//South
	else
	{
		if(Direction.X > 0) Zone = Southeast;
		else Zone = Southwest;	
	}
	return Zone;
}

#if WITH_EDITORONLY_DATA
void FPlayerRelativeWorldZoneConstraint::DrawConstraintDebug(UWorld* World, FLinearColor DebugColor, float ShowTime) const
{
	DrawOldConstraintDebug(World, Player->GetActorLocation(), DebugColor, ShowTime);
}

void FPlayerRelativeWorldZoneConstraint::DrawOldConstraintDebug(UWorld* World, const FVector& Position,
	FLinearColor DebugColor, float ShowTime) const
{
	const FVector Extent = FVector(1000.f, 1000.f, 50.f);
	FVector Offset = Extent;
	switch(ConstraintZone)
	{
	case Northwest:
		{
			Offset.X = -Offset.X;
		}
	case Northeast: break;
	case Southwest:
		{
			Offset.X = -Offset.X;
		}
	case Southeast:
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
