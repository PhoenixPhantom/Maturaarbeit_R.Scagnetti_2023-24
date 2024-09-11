// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/Animation/SuckToTargetComponent.h"

#include "Kismet/KismetMathLibrary.h"

DEFINE_LOG_CATEGORY(LogSuckToTarget);

FWarpInformation::FWarpInformation() : WarpType(EWarpType::LocationAndRotation), WarpSource(EWarpSource::None),
	TargetObject(nullptr), bFollowTarget(false), MaxWarpingDistance(-1.f), bMovementX(true),
    bMovementY(true), bMovementZ(false), bRotationPitch(false), bRotationYaw(true), bRotationRoll(false)
{
}

USuckToTargetComponent::USuckToTargetComponent() : RemainingWarpTime(0.f), TotalWarpTime(0.f),
	WarpSource(EWarpSource::None), TargetObject(nullptr), MaxWarpingDistance(-1.f), bWarpLocation(false),
	bMovementX(false), bMovementY(false), bMovementZ(false), bWarpRotation(false), bRotationPitch(false),
	bRotationYaw(false), bRotationRoll(false)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}


void USuckToTargetComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if(IsWarping())
	{
		const FTransform ResultingTransform = MotionWarpInternal(DeltaTime);
		GetOwner()->SetActorTransform(ResultingTransform, true);
	}
}

void USuckToTargetComponent::InterruptWarping(FInterruptMotionWarpingKey)
{
	PrimaryComponentTick.SetTickFunctionEnable(false);
}

void USuckToTargetComponent::SetOrUpdateWarpTarget(const FWarpInformation& WarpInformation)
{
	if(IsWarping()) PrimaryComponentTick.SetTickFunctionEnable(false);
	
	SetFromWarpingTypeInternal(WarpInformation);
	SetFromWarpingSourceInternal(WarpInformation);
	
}

FVector USuckToTargetComponent::GetTargetLocation() const
{
	const FVector& CurrentLocation = GetOwner()->GetActorLocation();
	if(!IsWarping() || !bWarpLocation) return CurrentLocation;
	if(TargetObject == nullptr)
	{
		return FVector(bMovementX ? TargetLocation.X : CurrentLocation.X, bMovementY ? TargetLocation.Y : CurrentLocation.Y,
			bMovementZ ? TargetLocation.Z : CurrentLocation.Z);
	}
	//else
	const FVector& ResultingTargetLocation = TargetObject->GetComponentLocation();
	return FVector(bMovementX ? ResultingTargetLocation.X : CurrentLocation.X,
		bMovementY ? ResultingTargetLocation.Y : CurrentLocation.Y, bMovementZ ? ResultingTargetLocation.Z : CurrentLocation.Z);
}

FVector USuckToTargetComponent::GetNextLocation(const FTransform& CurrentTransform, float DeltaSeconds) const
{
	const FVector CurrentLocation = CurrentTransform.GetLocation();
	if(!bWarpLocation) return CurrentLocation;
	FVector NewLocation;
	if(WarpSource == EWarpSource::FaceTargetObject || WarpSource == EWarpSource::MatchTargetObject)
	{
		if(!IsValid(TargetObject)) return CurrentLocation;
		const FTransform TargetTransform = GetTargetTransformFromComponent(TargetObject, TargetBoneName);
		NewLocation = FMath::VInterpTo(CurrentLocation, TargetTransform.GetLocation(),
			DeltaSeconds, 1.f/RemainingWarpTime);
		if(MaxWarpingDistance > 0.f)
		{
			FVector DesiredOffsetDirection;
			float DesiredOffsetDistance;
			(NewLocation - OriginalLocation).ToDirectionAndLength(DesiredOffsetDirection, DesiredOffsetDistance);
			if(DesiredOffsetDistance > MaxWarpingDistance)
			{
				NewLocation = OriginalLocation + DesiredOffsetDirection * MaxWarpingDistance;
			}
		}
	}
	else
	{
		NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation,
		DeltaSeconds, 1.f/RemainingWarpTime);
	}
	
	if(!bMovementX) NewLocation.X = CurrentLocation.X;
	if(!bMovementY) NewLocation.Y = CurrentLocation.Y;
	if(!bMovementZ) NewLocation.Z = CurrentLocation.Z;
	
	return NewLocation;
}

FRotator USuckToTargetComponent::GetNextRotation(const FTransform& CurrentTransform, float DeltaSeconds) const
{
	const FRotator CurrentRotation = CurrentTransform.Rotator();
	if(!bWarpRotation) return CurrentRotation;
	FRotator NewRotation;
	if(WarpSource == EWarpSource::FaceTargetObject)
	{
		if(!IsValid(TargetObject)) return CurrentRotation;
		const FVector TargetPosition = GetTargetTransformFromComponent(TargetObject, TargetBoneName).GetLocation();
		NewRotation = FMath::RInterpTo(CurrentRotation,
			UKismetMathLibrary::FindLookAtRotation(CurrentTransform.GetLocation(), TargetPosition),
			DeltaSeconds, 1.f/RemainingWarpTime);
	}
	else if(WarpSource == EWarpSource::MatchTargetObject)
	{
		if(!IsValid(TargetObject)) return CurrentRotation;
		const FRotator TargetRotator =
			GetTargetTransformFromComponent(TargetObject, TargetBoneName).GetRotation().Rotator();
		NewRotation =
			FMath::RInterpTo(CurrentRotation, TargetRotator,DeltaSeconds, 1.f/RemainingWarpTime);
	}
	else
	{
		NewRotation =
			FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaSeconds, 1.f/RemainingWarpTime);
	}
	
	if(!bRotationPitch)	NewRotation.Pitch = CurrentRotation.Pitch;
	if(!bRotationRoll) NewRotation.Roll = CurrentRotation.Roll;
	if(!bRotationYaw) NewRotation.Yaw = CurrentRotation.Yaw;
	
	return NewRotation;
}

FTransform USuckToTargetComponent::GetNextTransform(const FTransform& CurrentTransform, float DeltaSeconds) const
{
	return FTransform(GetNextRotation(CurrentTransform, DeltaSeconds),
		GetNextLocation(CurrentTransform, DeltaSeconds), CurrentTransform.GetScale3D());
}



void USuckToTargetComponent::StartWarpingInternal(float WarpTime)
{
	RemainingWarpTime = TotalWarpTime = WarpTime;
	if(MaxWarpingDistance > 0.f) OriginalLocation = GetOwner()->GetActorLocation();
	PrimaryComponentTick.SetTickFunctionEnable(true);
}

FTransform USuckToTargetComponent::MotionWarpInternal(float DeltaSeconds)
{
	const FTransform Transform = GetNextTransform(GetOwner()->GetActorTransform(), DeltaSeconds);
	RemainingWarpTime -= DeltaSeconds;
	if(!IsWarping()) PrimaryComponentTick.SetTickFunctionEnable(false);
	
#if WITH_EDITORONLY_DATA
	if(bIsDebugging) DrawDebugBox(GetWorld(), Transform.GetLocation(), Transform.GetScale3D() * 20.f,
		FColor(0, 255, 200));
#endif
	
	return Transform;
}

void USuckToTargetComponent::SetFromWarpingTypeInternal(const FWarpInformation& WarpInformation)
{
	check(WarpInformation.WarpType != EWarpType::None);
	
	bWarpLocation = WarpInformation.WarpType != EWarpType::RotationOnly;
	if(bWarpLocation)
	{
		bMovementX = WarpInformation.bMovementX;
		bMovementY = WarpInformation.bMovementY;
		bMovementZ = WarpInformation.bMovementZ;
	}
	
	bWarpRotation = WarpInformation.WarpType != EWarpType::LocationOnly;
	if(bWarpRotation)
	{
		bRotationPitch = WarpInformation.bRotationPitch;
		bRotationRoll = WarpInformation.bRotationRoll;
		bRotationYaw = WarpInformation.bRotationYaw;
	}
}

void USuckToTargetComponent::SetFromWarpingSourceInternal(const FWarpInformation& WarpInformation)
{
	check(WarpInformation.WarpSource != EWarpSource::None);
	WarpSource = WarpInformation.WarpSource;

	switch(WarpInformation.WarpSource)
	{
	case EWarpSource::FaceLocation:
		{
			SetFaceLocationInternal(WarpInformation);
			break;
		}
	case EWarpSource::MatchLocAndRot:
		{
			SetMatchLocAndRotInternal(WarpInformation);
			break;
		}
	case EWarpSource::FaceTargetObject:
		{
			SetFaceTargetObjectInternal(WarpInformation);
			break;
		}
		
	case EWarpSource::MatchTargetObject:
		{
			SetMatchTargetObjectInternal(WarpInformation);
			break;
		}
	default: checkNoEntry();
	}
}


void USuckToTargetComponent::SetFaceLocationInternal(const FWarpInformation& WarpInformation)
{
	TargetObject = nullptr;
	TargetBoneName = "";
	MaxWarpingDistance = -1.f;
	TargetLocation = WarpInformation.TargetLocation;
	TargetRotation = UKismetMathLibrary::FindLookAtRotation(GetOwner()->GetActorLocation(),
		WarpInformation.TargetLocation);
}

void USuckToTargetComponent::SetMatchLocAndRotInternal(const FWarpInformation& WarpInformation)
{
	TargetObject = nullptr;
	TargetBoneName = "";
	MaxWarpingDistance = -1.f;
	TargetLocation = WarpInformation.TargetLocation;
	TargetRotation = WarpInformation.TargetRotation;
}

void USuckToTargetComponent::SetFaceTargetObjectInternal(const FWarpInformation& WarpInformation)
{
	if(WarpInformation.bFollowTarget)
	{
		TargetObject = WarpInformation.TargetObject;
		TargetBoneName = WarpInformation.TargetBoneName;
		MaxWarpingDistance = WarpInformation.MaxWarpingDistance;
	}
	else
	{
		TargetObject = nullptr;
		TargetBoneName = "";
		MaxWarpingDistance = -1.f;
		const FTransform Transform = GetTargetTransformFromComponent(WarpInformation.TargetObject,
			WarpInformation.TargetBoneName);
		TargetLocation = Transform.GetLocation();
		TargetRotation = UKismetMathLibrary::FindLookAtRotation(GetOwner()->GetActorLocation(),
			Transform.GetLocation());
		WarpSource = EWarpSource::FaceLocation;
	}
}

void USuckToTargetComponent::SetMatchTargetObjectInternal(const FWarpInformation& WarpInformation)
{
	if(WarpInformation.bFollowTarget)
	{
		TargetObject = WarpInformation.TargetObject;
		TargetBoneName = WarpInformation.TargetBoneName;
		MaxWarpingDistance = WarpInformation.MaxWarpingDistance;
	}
	else
	{
		TargetObject = nullptr;
		TargetBoneName = "";
		MaxWarpingDistance = -1.f;
		const FTransform Transform = GetTargetTransformFromComponent(WarpInformation.TargetObject,
		WarpInformation.TargetBoneName);
		TargetLocation = Transform.GetLocation();
		TargetRotation = Transform.Rotator();
		WarpSource = EWarpSource::MatchLocAndRot;
	}
	
}

FTransform USuckToTargetComponent::GetTargetTransformFromComponent(const USceneComponent* TargetObject, FName TargetBoneName)
{
	if(!IsValid(TargetObject)) return FTransform::Identity;
	if(TargetBoneName == NAME_None) return TargetObject->GetComponentTransform();
	return TargetObject->GetSocketTransform(TargetBoneName);
}
