// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/Animation/SuckToTargetComponent.h"

#include "Kismet/KismetMathLibrary.h"

FWarpInformation::FWarpInformation() : WarpType(EWarpType::LocationAndRotation), WarpSource(EWarpSource::None), TargetObject(nullptr),
	bFollowTarget(false), bMovementX(true), bMovementY(true), bMovementZ(false), bRotationPitch(false),
	bRotationYaw(true), bRotationRoll(false)
{
}

USuckToTargetComponent::USuckToTargetComponent()
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
		if(ResultingTransform.ContainsNaN())
		{
			unimplemented(); //theoretically this shouldn't happen anymore
			/*if(bWarpLocation && ResultingTransform.GetLocation().ContainsNaN()) return;
			if(bWarpLocation && ResultingTransform.Rotator().ContainsNaN()) return;*/
		}
		GetOwner()->SetActorTransform(ResultingTransform, true);
	}
}

void USuckToTargetComponent::SetOrUpdateWarpTarget(const FWarpInformation& WarpInformation)
{
	if(IsWarping()) unimplemented(); //overriding or eventually blending warp targets isn't supported (yet)

	SetFromWarpingTypeInternal(WarpInformation);
	SetFromWarpingSourceInternal(WarpInformation);
	
}

FVector USuckToTargetComponent::GetNextLocation(const FTransform& CurrentTransform, float DeltaSeconds) const
{
	const FVector CurrentLocation = CurrentTransform.GetLocation();
	if(!bWarpLocation) return CurrentLocation;
	FVector NewLocation;
	if(WarpSource == EWarpSource::FaceTargetObject || WarpSource == EWarpSource::MatchTargetObject)
	{
		const FTransform TargetTransform = GetTargetTransformFromComponent(TargetObject, TargetBoneName);
		NewLocation = FMath::VInterpTo(CurrentLocation, TargetTransform.GetLocation(),
			DeltaSeconds, 1.f/RemainingWarpTime);
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
		const FVector TargetPosition = GetTargetTransformFromComponent(TargetObject, TargetBoneName).GetLocation();
		NewRotation = FMath::RInterpTo(CurrentRotation,
			UKismetMathLibrary::FindLookAtRotation(CurrentTransform.GetLocation(), TargetPosition),
			DeltaSeconds, 1.f/RemainingWarpTime);
	}
	else if(WarpSource == EWarpSource::MatchTargetObject)
	{
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
	TargetLocation = WarpInformation.TargetLocation;
	TargetRotation = UKismetMathLibrary::FindLookAtRotation(GetOwner()->GetActorLocation(),
		WarpInformation.TargetLocation);
}

void USuckToTargetComponent::SetMatchLocAndRotInternal(const FWarpInformation& WarpInformation)
{
	TargetObject = nullptr;
	TargetBoneName = "";
	TargetLocation = WarpInformation.TargetLocation;
	TargetRotation = WarpInformation.TargetRotation;
}

void USuckToTargetComponent::SetFaceTargetObjectInternal(const FWarpInformation& WarpInformation)
{
	if(WarpInformation.bFollowTarget)
	{
		TargetObject = WarpInformation.TargetObject;
		TargetBoneName = WarpInformation.TargetBoneName;
	}
	else
	{
		TargetObject = nullptr;
		TargetBoneName = "";
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
	}
	else
	{
		TargetObject = nullptr;
		TargetBoneName = "";
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
