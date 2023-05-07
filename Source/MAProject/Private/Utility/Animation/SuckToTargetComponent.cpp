// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/Animation/SuckToTargetComponent.h"

#include "Kismet/KismetMathLibrary.h"

USuckToTargetComponent::USuckToTargetComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void USuckToTargetComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if(IsWarping()) OwnerRef->SetActorTransform(MotionWarp(OwnerRef->GetActorTransform(), DeltaTime), true);
}

void USuckToTargetComponent::SetWarpTarget(USceneComponent* TargetComp, bool ShouldFollow, FName BoneName,
                                              bool AllowZAxisMovement, bool AllowOnlyYawRotation)
{
	StartWarpingInternal(-1.f); //make sure the warping doesn't start to early
	bAllowZAxisMovement = AllowZAxisMovement;
	bAllowOnlyYawRotation = AllowOnlyYawRotation;
	MotionWarpingTarget.Component = TargetComp;
	MotionWarpingTarget.BoneName = BoneName;
	MotionWarpingTarget.bFollowComponent = ShouldFollow;
	const FTransform Transform = GetTargetTransformFromComponent(TargetComp, BoneName);
	MotionWarpingTarget.Location = Transform.GetLocation();
	MotionWarpingTarget.Rotation = Transform.Rotator();
}

void USuckToTargetComponent::SetWarpTargetFaceTowards(USceneComponent* TargetComp, FVector CurrentLocation,
	FName BoneName, bool AllowZAxisMovement, bool AllowOnlyYawRotation)
{
	StartWarpingInternal(-1.f); //make sure the warping doesn't start to early
	bAllowZAxisMovement = AllowZAxisMovement;
	bAllowOnlyYawRotation = AllowOnlyYawRotation;
	MotionWarpingTarget.Component = TargetComp;
	MotionWarpingTarget.BoneName = BoneName;
	MotionWarpingTarget.bFollowComponent = false;
	MotionWarpingTarget.Location = GetTargetTransformFromComponent(TargetComp, BoneName).GetLocation();
	MotionWarpingTarget.Rotation = UKismetMathLibrary::FindLookAtRotation(CurrentLocation,
		MotionWarpingTarget.Location);
}
void USuckToTargetComponent::SetWarpTargetFaceTowards(USceneComponent* TargetComp, FName BoneName,
	bool AllowZAxisMovement, bool AllowOnlyYawRotation)
{
	StartWarpingInternal(-1.f); //make sure the warping doesn't start to early
	bAllowZAxisMovement = AllowZAxisMovement;
	bAllowOnlyYawRotation = AllowOnlyYawRotation;
	MotionWarpingTarget.Component = TargetComp;
	MotionWarpingTarget.BoneName = BoneName;
	MotionWarpingTarget.bFollowComponent = true;
	MotionWarpingTarget.Location = FVector(NAN);
	MotionWarpingTarget.Rotation = FRotator(NAN);
}

void USuckToTargetComponent::SetWarpTarget(FVector TargetLocation, FRotator TargetRotation, bool AllowZAxisMovement,
	bool AllowOnlyYawRotation)
{
	StartWarpingInternal(-1.f); //make sure the warping doesn't start to early
	bAllowZAxisMovement = AllowZAxisMovement;
	bAllowOnlyYawRotation = AllowOnlyYawRotation;
	MotionWarpingTarget.Component = nullptr;
	MotionWarpingTarget.BoneName = "";
	MotionWarpingTarget.bFollowComponent = false;
	MotionWarpingTarget.Location = TargetLocation;
	MotionWarpingTarget.Rotation = TargetRotation;
}

void USuckToTargetComponent::SetWarpTargetFaceTowards(FVector TargetLocation, FVector CurrentLocation,
	bool AllowZAxisMovement, bool AllowOnlyYawRotation)
{
	StartWarpingInternal(-1.f); //make sure the warping doesn't start to early
	bAllowZAxisMovement = AllowZAxisMovement;
	bAllowOnlyYawRotation = AllowOnlyYawRotation;
	MotionWarpingTarget.Component = nullptr;
	MotionWarpingTarget.BoneName = "";
	MotionWarpingTarget.bFollowComponent = false;
	MotionWarpingTarget.Location = TargetLocation;
	MotionWarpingTarget.Rotation = UKismetMathLibrary::FindLookAtRotation(CurrentLocation, TargetLocation);
}

FVector USuckToTargetComponent::GetNextLocation(const FTransform& CurrentTransform, float DeltaSeconds) const
{
	FVector Location = FMath::VInterpTo(CurrentTransform.GetLocation(),MotionWarpingTarget.GetLocation(),
		DeltaSeconds, 1.f/RemainingWarpTime);
	if(!bAllowZAxisMovement) Location.Z = CurrentTransform.GetLocation().Z;
	return Location;
}

FRotator USuckToTargetComponent::GetNextRotation(const FTransform& CurrentTransform, float DeltaSeconds) const
{
	FRotator Rotation = FMath::RInterpTo(CurrentTransform.Rotator(),
		MotionWarpingTarget.bFollowComponent && MotionWarpingTarget.Rotation.ContainsNaN() ?
		UKismetMathLibrary::FindLookAtRotation(CurrentTransform.GetLocation(),MotionWarpingTarget.GetLocation()) :
		MotionWarpingTarget.Rotator(), DeltaSeconds, 1.f/RemainingWarpTime);
	if(bAllowOnlyYawRotation)
	{
		Rotation.Roll = CurrentTransform.Rotator().Roll;
		Rotation.Pitch = CurrentTransform.Rotator().Pitch;
	}
	return Rotation;
}


FTransform USuckToTargetComponent::GetNextTransform(const FTransform& CurrentTransform, float DeltaSeconds) const
{
	return FTransform(GetNextRotation(CurrentTransform, DeltaSeconds),
		GetNextLocation(CurrentTransform, DeltaSeconds), CurrentTransform.GetScale3D());
}

FTransform USuckToTargetComponent::MotionWarp(const FTransform& CurrentTransform, float DeltaSeconds)
{
	const FTransform Transform = GetNextTransform(CurrentTransform, DeltaSeconds);
#if WITH_EDITORONLY_DATA
	if(bIsDebugging) DrawDebugBox(GetWorld(), Transform.GetLocation(), Transform.GetScale3D()*20.f,
		FColor(0, 255, 200));
#endif
	RemainingWarpTime -= DeltaSeconds;
	return Transform;
}

FTransform USuckToTargetComponent::GetTargetTransformFromComponent(const USceneComponent* Component, FName BoneName)
{
	if(!IsValid(Component)) return FTransform::Identity;
	if(BoneName == NAME_None) return Component->GetComponentTransform();
	return Component->GetSocketTransform(BoneName);
}

void USuckToTargetComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerRef = GetOwner();
	check(IsValid(OwnerRef));
}

