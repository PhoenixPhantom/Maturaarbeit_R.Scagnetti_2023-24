// Fill out your copyright notice in the Description page of Project Settings.


#include "NPCRelativeConstraints.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

void DrawDebugCircularFrustum(UWorld* World, const FVector& Center, const FVector& Direction, float Radius1, float Radius2,
	int32 NumSegments,
	FLinearColor DebugColor, float ShowTime)
{
#if ENABLE_DRAW_DEBUG
	const FVector Circle1Origin = Center + Direction*0.5f;
	const FVector Circle2Origin = Center - Direction*0.5f;
	const FRotator Rotation = (Direction - FVector(1.f, 1.f, 1.f)).Rotation();
	UKismetSystemLibrary::DrawDebugCircle(World, Circle1Origin, Radius1, NumSegments, DebugColor, ShowTime,
		0.f, Rotation.RotateVector(FVector(0.f, 1.f, 0.f)),
			Rotation.RotateVector(FVector(0.f, 0.f, 1.f)));
	UKismetSystemLibrary::DrawDebugCircle(World, Circle2Origin, Radius2, NumSegments, DebugColor, ShowTime,
	0.f, Rotation.RotateVector(FVector(0.f, 1.f, 0.f)),
		Rotation.RotateVector(FVector(0.f, 0.f, 1.f)));

	FVector RotationAxis = Direction;
	RotationAxis.Normalize();
	const FVector OffsetDirection = Rotation.RotateVector(FVector(0.f, 0.f, 1.f));
	for(int32 i = 0; i < NumSegments; i++)
	{
		const FVector CurrentOffsetDirection =
			OffsetDirection.RotateAngleAxisRad(DOUBLE_PI*2.0/static_cast<double>(NumSegments)*static_cast<double>(i), RotationAxis);
		UKismetSystemLibrary::DrawDebugLine(World, Circle1Origin + CurrentOffsetDirection * Radius1,
			Circle2Origin + CurrentOffsetDirection * Radius2, DebugColor, ShowTime);
	}
#endif
}

#if WITH_EDITORONLY_DATA
void FNonCombatConstraint::DrawConstraintDebug(UWorld* World, FLinearColor DebugColor, float ShowTime) const
{
	UKismetSystemLibrary::DrawDebugSphere(World, Owner->GetActorLocation() + PositionOffset, Radius,
		50, DebugColor, ShowTime);
}
#endif

#if WITH_EDITORONLY_DATA
void FActiveCombatConstraint::DrawConstraintDebug(UWorld* World, FLinearColor DebugColor, float ShowTime) const
{
	UKismetSystemLibrary::DrawDebugSphere(World, Owner->GetActorLocation() + PositionOffset, Radius,
		50, DebugColor, ShowTime);
}
#endif

FPassiveCombatConstraint::FPassiveCombatConstraint() : FNpcRelativeConstraints(), OrientationCenter(nullptr),
                                                       VerticalSize(100.f), HorizontalSize(100.f)
{}

FPassiveCombatConstraint::FPassiveCombatConstraint(AActor* SourceNpc, AActor* SourceOrientationCenter) :
	FNpcRelativeConstraints(SourceNpc), OrientationCenter(SourceOrientationCenter), VerticalSize(100.f),
	HorizontalSize(100.f)
{}

bool FPassiveCombatConstraint::IsConstraintSatisfied(FVector Position) const
{
	FVector Direction;
	float OwnDistanceFromCenter;
	((Owner->GetActorLocation() + PositionOffset) - OrientationCenter->GetActorLocation()).ToDirectionAndLength(Direction,
		OwnDistanceFromCenter);
	const float ActualRadius = OwnDistanceFromCenter - VerticalSize/2.f;
	const float DotProduct = FVector::DotProduct(Direction,
	            UKismetMathLibrary::GetDirectionUnitVector(OrientationCenter->GetActorLocation(), Position));

	const float ConstrainedAngle = 2.f * PI * std::min(HorizontalSize / (2.f * PI * OwnDistanceFromCenter), 1.f);

	const float PositionDistanceFromCenter = FVector::Distance(OrientationCenter->GetActorLocation(), Position);
	return UKismetMathLibrary::Acos(DotProduct) > ConstrainedAngle &&
		(PositionDistanceFromCenter < ActualRadius || PositionDistanceFromCenter > (ActualRadius + VerticalSize));
}

#if WITH_EDITORONLY_DATA
void FPassiveCombatConstraint::DrawConstraintDebug(UWorld* World, FLinearColor DebugColor, float ShowTime) const
{
	FVector Direction;
	float DistanceFromCenter;
	const FVector FrustumLocation = Owner->GetActorLocation() + PositionOffset;
	(FrustumLocation - OrientationCenter->GetActorLocation()).ToDirectionAndLength(Direction,
	DistanceFromCenter);
	//const float ActualRadius = DistanceFromCenter - VerticalSize/2.f;
	const float Angle = 2.f * PI * std::min(HorizontalSize / (2.f * PI * DistanceFromCenter), 1.f);
	DrawDebugCircularFrustum(World,	Owner->GetActorLocation() + PositionOffset, Direction * VerticalSize,
		(DistanceFromCenter + VerticalSize/2.f)*sinf(Angle),
		(DistanceFromCenter - VerticalSize/2.f)*sinf(Angle), 20, DebugColor,
		ShowTime);
}
#endif
