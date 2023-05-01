// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputManagement.h"
#include "GameFramework/Character.h"
#include "GeneralCharacter.generated.h"

class UMotionWarpingComponent;

UCLASS()
class AGeneralCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGeneralCharacter();

	virtual float GetFieldOfView() const { unimplemented(); return 0.f; }

	/**
	 * @brief Get weather an input of the given can override the currently active one or not
	 * @param Type the input to check
	 * @return Weather the current input can be overridden by Type
	 */
	bool CanOverrideCurrentInput(EInputType Type) const { return AcceptedInputs.CanOverrideCurrentInput(Type); }

	virtual void GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;

protected:
	FAcceptedInputs AcceptedInputs;
	
	//The prefix (if existent) every bone on the characters skeleton has
	UPROPERTY(EditAnywhere, Category="Base Settings")
	FString BonePrefix;
	UPROPERTY(EditAnywhere)
	UMotionWarpingComponent* MotionWarpingComponent;

#if WITH_EDITORONLY_DATA
	float OldFieldOfView;
#endif
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
