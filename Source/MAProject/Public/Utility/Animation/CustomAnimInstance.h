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
	void SetIsInAir(bool IsInAir){ bIsInAir = IsInAir; }

	bool IsInState(ECustomAnimationState State) const;
	UFUNCTION(BlueprintCallable, Category = States, meta=(TargetSatate="CustomState0"))
	void EnterCustomState(ECustomAnimationState TargetState);
	UFUNCTION(BlueprintCallable, Category = States, meta=(TargetSatate="CustomState0"))
	void ExitCustomState(ECustomAnimationState TargetState);

	float GetDeathAnimTime() const{ return DeathAnimTime; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float MovementSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	FVector RelativeMovementDirection;

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
	
};
