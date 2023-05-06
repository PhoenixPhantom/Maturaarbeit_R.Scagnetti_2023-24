// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RootMotionModifier.h"
#include "Components/ActorComponent.h"
#include "SuckToTargetComponent.generated.h"

struct FStartMotionWarpingKey final
{
	friend class USuckToTargetNotifyState;
private:
		FStartMotionWarpingKey(){}
};

UCLASS(ClassGroup=(Custom))
class MAPROJECT_API USuckToTargetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USuckToTargetComponent();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	FORCEINLINE void StartWarping(float WarpTime, FStartMotionWarpingKey Key){ StartWarpingInternal(WarpTime); }

	FORCEINLINE bool IsWarping() const { return RemainingWarpTime > 0.f; }

	//Sets the warp target and warps the rotation to MATCH the one of TargetComp
	void SetWarpTarget(USceneComponent* TargetComp, bool ShouldFollow = false, FName BoneName = NAME_None,
		bool AllowZAxisMovement = false, bool AllowOnlyYawRotation = true);
	//Sets the warp target and warps the rotation to FACE TargetComp (doesn't follow TargetComp)
	void SetWarpTargetFaceTowards(USceneComponent* TargetComp, FVector CurrentLocation, FName BoneName = NAME_None,
		bool AllowZAxisMovement = false, bool AllowOnlyYawRotation = true);
	//Sets the warp target and warps the rotation to FACE TargetComp (follows TargetComp)
	void SetWarpTargetFaceTowards(USceneComponent* TargetComp, FName BoneName = NAME_None,
		bool AllowZAxisMovement = false, bool AllowOnlyYawRotation = true);
	void SetWarpTarget(FVector TargetLocation, FRotator TargetRotation, bool AllowZAxisMovement = false,
		bool AllowOnlyYawRotation = true);
	void SetWarpTargetFaceTowards(FVector TargetLocation, FVector CurrentLocation, bool AllowZAxisMovement = false,
		bool AllowOnlyYawRotation = true);
	
	FVector GetNextLocation(const FTransform& CurrentTransform, float DeltaSeconds) const;
	FRotator GetNextRotation(const FTransform& CurrentTransform, float DeltaSeconds) const;
	FTransform GetNextTransform(const FTransform& CurrentTransform, float DeltaSeconds) const;
	
	FTransform MotionWarp(const FTransform& CurrentTransform, float DeltaSeconds);

#if WITH_EDITORONLY_DATA
	bool bIsDebugging = false;
#endif
	
protected:
	float RemainingWarpTime;
	float TotalWarpTime;
	bool bAllowZAxisMovement;
	bool bAllowOnlyYawRotation;
	FMotionWarpingTarget MotionWarpingTarget;

	FORCEINLINE void StartWarpingInternal(float WarpTime){ RemainingWarpTime = TotalWarpTime = WarpTime; }
};
