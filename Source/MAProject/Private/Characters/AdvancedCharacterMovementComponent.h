// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AdvancedCharacterMovementComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MAPROJECT_API UAdvancedCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UAdvancedCharacterMovementComponent();

	virtual FRotator ComputeOrientToMovementRotation(const FRotator& CurrentRotation, float DeltaTime,
		FRotator& DeltaRotation) const override;

	virtual void SetWalkBackwards(bool WalkBackwards){ bWalkBackwards = WalkBackwards; }

protected:
	uint8 bWalkBackwards:1;
};
