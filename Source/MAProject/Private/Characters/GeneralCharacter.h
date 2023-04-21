// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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

	float GetFieldOfView() const { return FieldOfView; }

	/**
	 * @brief Get weather an input of the given can override the currently active one or not
	 * @param Type the input to check
	 * @return Weather the current input can be overridden by Type
	 */
	bool CanOverrideCurrentInput(EInputType Type) const { return CurrentlyAvailableInputs.CanOverrideCurrentInput(Type); }

	virtual void GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	FAvailableInputs CurrentlyAvailableInputs;
	
	//Central place to set the field of view (so these settings are done the same way for all characters)
	UPROPERTY(EditDefaultsOnly, Category="Base Settings")
	float FieldOfView;

	//The prefix (if existent) every bone on the characters skeleton has
	UPROPERTY(EditAnywhere, Category="Base Settings")
	FString BonePrefix;

	//TODO: maybe add physical animation component for hit and death simulations

	UPROPERTY(EditAnywhere)
	UMotionWarpingComponent* MotionWarpingComponent;

	//TODO: add OnGetHit & OnGetStaggered central implementation
};
