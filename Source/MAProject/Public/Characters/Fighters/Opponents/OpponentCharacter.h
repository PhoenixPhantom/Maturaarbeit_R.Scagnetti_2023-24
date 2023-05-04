// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Fighters/FighterCharacter.h"
#include "Perception/PawnSensingComponent.h"
#include "OpponentCharacter.generated.h"

class USavableObjectMarkerComponent;
class UTargetInformationComponent;
/**
 * 
 */
UCLASS()
class MAPROJECT_API AOpponentCharacter : public AFighterCharacter
{
	GENERATED_BODY()
public:
	AOpponentCharacter();
	virtual float GetFieldOfView() const override;

protected:
	UPROPERTY(EditAnywhere)
	USavableObjectMarkerComponent* SavableObjectMarkerComponent;

	UPROPERTY(EditAnywhere)
	UTargetInformationComponent* TargetInformationComponent;

	UPROPERTY(SaveGame)
	FSavableCharacterModifiers StatsModifiers;

	virtual void BeginPlay() override;
};
