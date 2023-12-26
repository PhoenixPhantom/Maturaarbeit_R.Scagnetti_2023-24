// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SuckToTargetComponent.generated.h"

struct FStartMotionWarpingKey final
{
	friend class UAnimNotifyState_SuckToTarget;
private:
		FStartMotionWarpingKey(){}
};

struct FInterruptMotionWarpingKey final
{
	friend class APlayerCharacter;
private:
		FInterruptMotionWarpingKey(){}
};

enum class EWarpType : uint8
{
	None,
	LocationOnly,
	RotationOnly,
	LocationAndRotation
};

enum class EWarpSource : uint8
{
	None,
	FaceLocation,
	MatchLocAndRot,
	FaceTargetObject,
	MatchTargetObject
};

DECLARE_LOG_CATEGORY_EXTERN(LogSuckToTarget, Log, Warning);

struct FWarpInformation
{
	FWarpInformation();
	//Determines what parts of the transform will be modified
	EWarpType WarpType;
	//Sets which given parameters will be used to influence the resulting transform
	EWarpSource WarpSource;

	FVector TargetLocation;
	FRotator TargetRotation;
	
	USceneComponent* TargetObject;
	FName TargetBoneName;
	uint8 bFollowTarget:1;
	//The max distance change the warping can make
	//(ONLY works WarpSource = EWarpSource::FaceTargetObject/MatchTargetObject)
	float MaxWarpingDistance;

	uint8 bMovementX:1;
	uint8 bMovementY:1;
	uint8 bMovementZ:1;
	//Around Y axis
	uint8 bRotationPitch:1;
	//Around Z axis
	uint8 bRotationYaw:1;
	//around X axis
	uint8 bRotationRoll:1;
};

UCLASS(ClassGroup=(Custom))
class MAPROJECT_API USuckToTargetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USuckToTargetComponent();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	FORCEINLINE void StartWarping(float WarpTime, FStartMotionWarpingKey){ StartWarpingInternal(WarpTime); }
	FORCEINLINE void InterruptWarping(FInterruptMotionWarpingKey);

	FORCEINLINE bool IsWarping() const { return RemainingWarpTime > 0.f; }

	void SetOrUpdateWarpTarget(const FWarpInformation& WarpInformation);

	FVector GetTargetLocation() const;	
	FVector GetNextLocation(const FTransform& CurrentTransform, float DeltaSeconds) const;
	FRotator GetNextRotation(const FTransform& CurrentTransform, float DeltaSeconds) const;
	FTransform GetNextTransform(const FTransform& CurrentTransform, float DeltaSeconds) const;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = Debugging)
	bool bIsDebugging = false;
#endif
	
protected:
	float RemainingWarpTime;
	float TotalWarpTime;

	EWarpSource WarpSource;
	
	FVector TargetLocation;
	FRotator TargetRotation;
	FVector OriginalLocation;

	UPROPERTY()
	USceneComponent* TargetObject;
	FName TargetBoneName;
	float MaxWarpingDistance;
	
	uint8 bWarpLocation:1;
	uint8 bMovementX:1;
	uint8 bMovementY:1;
	uint8 bMovementZ:1;
	
	uint8 bWarpRotation:1;
	//Around Y axis
	uint8 bRotationPitch:1;
	//Around Z axis
	uint8 bRotationYaw:1;
	//around X axis
	uint8 bRotationRoll:1;


	FORCEINLINE void StartWarpingInternal(float WarpTime);
	FTransform MotionWarpInternal(float DeltaSeconds);

	void SetFromWarpingTypeInternal(const FWarpInformation& WarpInformation);
	void SetFromWarpingSourceInternal(const FWarpInformation& WarpInformation);
	
	void SetFaceLocationInternal(const FWarpInformation& WarpInformation);
	void SetMatchLocAndRotInternal(const FWarpInformation& WarpInformation);
	void SetFaceTargetObjectInternal(const FWarpInformation& WarpInformation);
	void SetMatchTargetObjectInternal(const FWarpInformation& WarpInformation);

	static FTransform GetTargetTransformFromComponent(const USceneComponent* Component, FName BoneName);
};