// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Fighters/FighterCharacter.h"
#include "Characters/Fighters/Player/PlayerCharacter.h"
#include "OpponentCharacter.generated.h"

class ACombatManager;
class AOpponentController;
class USavableObjectMarkerComponent;
class UTargetInformationComponent;

struct FSetFieldOfViewKey final
{
	friend AOpponentController;
private:
	FSetFieldOfViewKey(){};
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAggressionTokenGrantedDelegate);

/**
 * 
 */
UCLASS()
class MAPROJECT_API AOpponentCharacter : public AFighterCharacter
{
	GENERATED_BODY()
public:
	FOnAggressionTokenGrantedDelegate OnAggressionTokensGranted;
	
	AOpponentCharacter();
	virtual float GetFieldOfView() const override { return LocalFieldOfView; };

	uint32 GetRequestedTokens() const { return RequestedAggressionTokens; }
	void SetLocalFieldOfView(float FieldOfView, FSetFieldOfViewKey Key){ LocalFieldOfView = FieldOfView; };

	/**
	 * @brief Generate a score for the importance of the opponent to the player. Screen centered-ness,
	 * distance from player and opponent preferences as well as opponent importance are taken into account
	 * @param PlayerCharacter The player in regards to which the score is generated
	 * @return the score >= 0.f if the opponent is allowed to become aggressive*/
	 float GenerateAggressionScore(APlayerCharacter* PlayerCharacter);

protected:
	uint8 bIsAggressive:1;
	uint8 bCanBecomeAggressive:1;
	uint8 bIsCurrentTarget:1;

	float LocalFieldOfView;
	
	UPROPERTY(EditAnywhere)
	USavableObjectMarkerComponent* SavableObjectMarkerComponent;

	UPROPERTY(EditAnywhere)
	UTargetInformationComponent* TargetInformationComponent;

	UPROPERTY(SaveGame)
	FSavableCharacterModifiers StatsModifiers;

	UPROPERTY(EditAnywhere, Category=AI)
	uint32 RequestedAggressionTokens;
	
	UPROPERTY(EditAnywhere, Category=AI)
	float AggressionPriority;

	UPROPERTY(EditAnywhere, Category=AI, AdvancedDisplay)
	float AggressionRange;

	virtual void BeginPlay() override;
	
};