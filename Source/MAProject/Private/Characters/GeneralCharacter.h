// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputManagement.h"
#include "RootMotionModifier.h"
#include "GameFramework/Character.h"
#include "GeneralCharacter.generated.h"

class UMotionWarpingComponent;


struct FMotionWarpingInformation
{
	FMotionWarpingInformation() : RemainingWarpTime(-1.f), TotalWarpTime(0.f), bAllowZAxisMovement(false),
		bAllowOnlyYawRotation(true){}

	void StartWarping(float WarpTime){ RemainingWarpTime = TotalWarpTime = WarpTime; }

	bool IsWarping() const { return RemainingWarpTime > 0.f; }

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
	UWorld* World = nullptr;
#endif
	
protected:
	float RemainingWarpTime;
	float TotalWarpTime;
	bool bAllowZAxisMovement;
	bool bAllowOnlyYawRotation;
	FMotionWarpingTarget MotionWarpingTarget;
};


struct FMotionWarpingKey final
{
	friend class UCustomMotionWarpingNotifyState;
private:
	FMotionWarpingKey(){}
};


UCLASS()
class AGeneralCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGeneralCharacter();

	virtual float GetFieldOfView() const { unimplemented(); return 0.f; }
	
	void StartMotionWarping(float WarpingTime, FMotionWarpingKey Key){ MotionWarpingInformation.StartWarping(WarpingTime); }

	/**
	 * @brief Get weather an input of the given can override the currently active one or not
	 * @param Type the input to check
	 * @return Weather the current input can be overridden by Type
	 */
	bool CanOverrideCurrentInput(EInputType Type) const { return AcceptedInputs.CanOverrideCurrentInput(Type); }

	virtual void GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;

protected:
	FAcceptedInputs AcceptedInputs;
	FMotionWarpingInformation MotionWarpingInformation;
	
	//The prefix (if existent) every bone on the characters skeleton has
	UPROPERTY(EditAnywhere, Category= Character)
	FString BonePrefix;

#if WITH_EDITORONLY_DATA
	float OldFieldOfView;
#endif
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
