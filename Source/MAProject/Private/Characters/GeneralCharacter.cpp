// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/GeneralCharacter.h"
#include "MotionWarpingComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "MAProject/MAProject.h"


void FMotionWarpingInformation::SetWarpTarget(USceneComponent* TargetComp, bool ShouldFollow, FName BoneName,
	bool AllowZAxisMovement, bool AllowOnlyYawRotation)
{
	StartWarping(-1.f); //make sure the warping doesn't start to early
	bAllowZAxisMovement = AllowZAxisMovement;
	bAllowOnlyYawRotation = AllowOnlyYawRotation;
	MotionWarpingTarget.Component = TargetComp;
	MotionWarpingTarget.BoneName = BoneName;
	MotionWarpingTarget.bFollowComponent = ShouldFollow;
	if (BoneName != NAME_None)
	{
		const FTransform Transform = FMotionWarpingTarget::GetTargetTransformFromComponent(TargetComp, BoneName);
		MotionWarpingTarget.Location = Transform.GetLocation();
		MotionWarpingTarget.Rotation = Transform.Rotator();
	}
	else
	{
		MotionWarpingTarget.Location = TargetComp->GetComponentLocation();
		MotionWarpingTarget.Rotation = TargetComp->GetComponentRotation();
	}
}

void FMotionWarpingInformation::SetWarpTargetFaceTowards(USceneComponent* TargetComp, FVector CurrentLocation,
	FName BoneName, bool AllowZAxisMovement, bool AllowOnlyYawRotation)
{
	StartWarping(-1.f); //make sure the warping doesn't start to early
	bAllowZAxisMovement = AllowZAxisMovement;
	bAllowOnlyYawRotation = AllowOnlyYawRotation;
	MotionWarpingTarget.Component = TargetComp;
	MotionWarpingTarget.BoneName = BoneName;
	MotionWarpingTarget.bFollowComponent = false;
	if (BoneName != NAME_None)
	{
		MotionWarpingTarget.Location = FMotionWarpingTarget::GetTargetTransformFromComponent(TargetComp, BoneName).GetLocation();
	}
	else MotionWarpingTarget.Location = TargetComp->GetComponentLocation();
	MotionWarpingTarget.Rotation = UKismetMathLibrary::FindLookAtRotation(CurrentLocation, MotionWarpingTarget.Location);
}
void FMotionWarpingInformation::SetWarpTargetFaceTowards(USceneComponent* TargetComp, FName BoneName,
	bool AllowZAxisMovement, bool AllowOnlyYawRotation)
{
	StartWarping(-1.f); //make sure the warping doesn't start to early
	bAllowZAxisMovement = AllowZAxisMovement;
	bAllowOnlyYawRotation = AllowOnlyYawRotation;
	MotionWarpingTarget.Component = TargetComp;
	MotionWarpingTarget.BoneName = BoneName;
	MotionWarpingTarget.bFollowComponent = true;
	MotionWarpingTarget.Location = FVector(NAN);
	MotionWarpingTarget.Rotation = FRotator(NAN);
}

void FMotionWarpingInformation::SetWarpTarget(FVector TargetLocation, FRotator TargetRotation, bool AllowZAxisMovement,
	bool AllowOnlyYawRotation)
{
	StartWarping(-1.f); //make sure the warping doesn't start to early
	bAllowZAxisMovement = AllowZAxisMovement;
	bAllowOnlyYawRotation = AllowOnlyYawRotation;
	MotionWarpingTarget.Component = nullptr;
	MotionWarpingTarget.BoneName = "";
	MotionWarpingTarget.bFollowComponent = false;
	MotionWarpingTarget.Location = TargetLocation;
	MotionWarpingTarget.Rotation = TargetRotation;
}

void FMotionWarpingInformation::SetWarpTargetFaceTowards(FVector TargetLocation, FVector CurrentLocation,
	bool AllowZAxisMovement, bool AllowOnlyYawRotation)
{
	StartWarping(-1.f); //make sure the warping doesn't start to early
	bAllowZAxisMovement = AllowZAxisMovement;
	bAllowOnlyYawRotation = AllowOnlyYawRotation;
	MotionWarpingTarget.Component = nullptr;
	MotionWarpingTarget.BoneName = "";
	MotionWarpingTarget.bFollowComponent = false;
	MotionWarpingTarget.Location = TargetLocation;
	MotionWarpingTarget.Rotation = UKismetMathLibrary::FindLookAtRotation(CurrentLocation, TargetLocation);
}

FVector FMotionWarpingInformation::GetNextLocation(const FTransform& CurrentTransform, float DeltaSeconds) const
{
	FVector Location = FMath::VInterpTo(CurrentTransform.GetLocation(),MotionWarpingTarget.GetLocation(),
		DeltaSeconds, 1.f/RemainingWarpTime);
	if(!bAllowZAxisMovement) Location.Z = CurrentTransform.GetLocation().Z;
	return Location;
}

FRotator FMotionWarpingInformation::GetNextRotation(const FTransform& CurrentTransform, float DeltaSeconds) const
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


FTransform FMotionWarpingInformation::GetNextTransform(const FTransform& CurrentTransform, float DeltaSeconds) const
{
	return FTransform(GetNextRotation(CurrentTransform, DeltaSeconds),
		GetNextLocation(CurrentTransform, DeltaSeconds), CurrentTransform.GetScale3D());
}

FTransform FMotionWarpingInformation::MotionWarp(const FTransform& CurrentTransform, float DeltaSeconds)
{
	const FTransform Transform = GetNextTransform(CurrentTransform, DeltaSeconds);
#if WITH_EDITORONLY_DATA
	if(bIsDebugging) DrawDebugBox(World, Transform.GetLocation(), Transform.GetScale3D()*20.f,
		FColor(0, 255, 200));
#endif
	RemainingWarpTime -= DeltaSeconds;
	return Transform;
}

// Sets default values
AGeneralCharacter::AGeneralCharacter()
{
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionObjectType(ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECR_Overlap);

	GetMesh()->ComponentTags.Add(HitReactingVolumeTag);
	
}

void AGeneralCharacter::GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	if(!IsValid(GetMesh()))
	{
		Super::GetActorEyesViewPoint(OutLocation, OutRotation);
		return;
	}
	const FName HeadSocket = *(BonePrefix + "-HeadSocket");
	OutLocation = GetMesh()->GetSocketLocation(HeadSocket);
	OutRotation = GetMesh()->GetSocketRotation(HeadSocket);
}

// Called when the game starts or when spawned
void AGeneralCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

