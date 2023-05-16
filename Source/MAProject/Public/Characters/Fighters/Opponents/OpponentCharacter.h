// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Fighters/FighterCharacter.h"
#include "OpponentCharacter.generated.h"

class USavableObjectMarkerComponent;
class UTargetInformationComponent;


DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(float, FOnGenerateAggressionScoreDelegate, AOpponentCharacter*, TargetCharacter);

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

	float GenerateAggressionScore();
	float GetAggressionPriority(float Distance) const { return AggressionPriority * (1.f - Distance/AggressionRange); }

protected:
	uint8 bIsAggressive:1;
	uint8 bCanBecomeAggressive:1;
	uint8 bIsCurrentTarget:1;

	FOnGenerateAggressionScoreDelegate OnGenerateAggressionScore;
	
	UPROPERTY(EditAnywhere)
	USavableObjectMarkerComponent* SavableObjectMarkerComponent;

	UPROPERTY(EditAnywhere)
	UTargetInformationComponent* TargetInformationComponent;

	UPROPERTY(SaveGame)
	FSavableCharacterModifiers StatsModifiers;

	UPROPERTY(EditAnywhere, Category=AI)
	float AggressionPriority;

	UPROPERTY(EditAnywhere, Category=AI, AdvancedDisplay)
	float AggressionRange;

	virtual void BeginPlay() override;
};