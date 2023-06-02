// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerRelativeConstraints.h"

bool FPlayerDistanceConstraint::IsConstraintSatisfied(FVector Position) const
{
	const float Distance = FVector::Distance(Position, Player->GetActorLocation());
	return Distance <= MaxRadius && Distance >= MinRadius;
}

FPlayerRelativeWorldZoneConstraint::EWorldConstraintZone FPlayerRelativeWorldZoneConstraint::CalculateTargetZone(
	FVector TargetPosition) const
{
	FVector2d Direction = FVector2d(TargetPosition.X, TargetPosition.Y) - FVector2d(Player->GetActorLocation().X, Player->GetActorLocation().Y);
	Direction.Normalize();
	EWorldConstraintZone Zone;
	
	//North
	if(Direction.X > 0)
	{
		if(Direction.Y > 0) Zone = Northeast;
		else Zone = Northwest;
	}
	//South
	else
	{
		if(Direction.Y > 0) Zone = Southeast;
		else Zone = Southwest;	
	}
	return Zone;
}
