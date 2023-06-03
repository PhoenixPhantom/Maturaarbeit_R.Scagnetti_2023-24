// Fill out your copyright notice in the Description page of Project Settings.


#include "NPCRelativeConstraints.h"

#include "Kismet/KismetMathLibrary.h"

FPassiveCombatConstraint::FPassiveCombatConstraint() : FNpcRelativeConstraint(), OrientationCenter(nullptr),
	VerticalSize(100.f), HorizontalSize(100.f)
{}

FPassiveCombatConstraint::FPassiveCombatConstraint(AActor* SourceNpc, AActor* SourceOrientationCenter) :
	FNpcRelativeConstraint(SourceNpc), OrientationCenter(SourceOrientationCenter), VerticalSize(100.f),
	HorizontalSize(100.f)
{}

bool FPassiveCombatConstraint::IsConstraintSatisfied(FVector Position) const
{
	FVector Direction;
	float DistanceFromCenter;
	((Npc->GetActorLocation() + PositionOffset) - OrientationCenter->GetActorLocation()).ToDirectionAndLength(Direction,
		DistanceFromCenter);
	const float ActualRadius = DistanceFromCenter - VerticalSize/2.f;
	const float DotProduct = FVector::DotProduct(Direction,
	            UKismetMathLibrary::GetDirectionUnitVector(OrientationCenter->GetActorLocation(), Position));

	const float ConstrainedAngle = 2.f * PI * std::min(HorizontalSize / (ActualRadius + VerticalSize/2.f), 1.f);
	
	return (DistanceFromCenter < ActualRadius || DistanceFromCenter > (ActualRadius + VerticalSize)) &&
		UKismetMathLibrary::Acos(DotProduct) <= ConstrainedAngle;;
}
