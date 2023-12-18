// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CustomAnimInstance.generated.h"

UENUM(BlueprintType, meta=(Bitflags, UseEnumValuesAsMaskValuesInEditor="true"))
enum class ECustomAnimationState : uint8
{
	None = 0,
	CustomState0 = 1 << 0,
	CustomState1 = 1 << 1,
	CustomState2 = 1 << 2
};
ENUM_CLASS_FLAGS(ECustomAnimationState)

UENUM(BlueprintType, meta=(Bitflags, UseEnumValuesAsMaskValuesInEditor="true"))
enum class ELegIKType : uint8
{
	None = 0,
	LeftFront = 1 << 0,
	RightFront = 1 << 1,
	LeftBack = 1 << 2,
	RightBack = 1 << 3
};
ENUM_CLASS_FLAGS(ELegIKType)

/**
 * 
 */
UCLASS()
class MAPROJECT_API UCustomAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UCustomAnimInstance();
	void SetMovement(const FVector& MovementVector);
	void TriggerDeath(){ bIsDying = true; }
	void SetLegIKBlend(float NewBlend){ LegIKBlend = NewBlend; }
	void SetIsInAir(bool IsInAir){ bIsInAir = IsInAir; }

	bool IsInState(ECustomAnimationState State) const;
	UFUNCTION(BlueprintCallable, Category = States)
	void SetAllowedLegIKTypes(int32 AllowedTypes, float Duration);
	UFUNCTION(BlueprintCallable, Category = States, meta=(TargetSatate="CustomState0"))
	void EnterCustomState(ECustomAnimationState TargetState);
	UFUNCTION(BlueprintCallable, Category = States, meta=(TargetSatate="CustomState0"))
	void ExitCustomState(ECustomAnimationState TargetState);

	float GetDeathAnimTime() const{ return DeathAnimTime; }

protected:
	FTimerHandle AllowedIKTypesResetHandle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float MovementSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	FVector RelativeMovementDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = States)
	float LegIKBlend;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = States)
	float DeathAnimTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = States)
	uint8 bIsDying:1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = States)
	uint8 bIsInAir:1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = States)
	uint8 bIsInCustomState0:1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = States)
	uint8 bIsInCustomState1:1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = States)
	uint8 bIsInCustomState2:1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "States|IK")
	uint8 bAllowLeftFrontLeg:1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "States|IK")
	uint8 bAllowRightFrontLeg:1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "States|IK")
	uint8 bAllowLeftBackLeg:1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "States|IK")
	uint8 bAllowRightBackLeg:1;
	
};
