// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Fighters/FighterCharacter.h"
#include "Perception/PawnSensingComponent.h"
#include "OpponentCharacter.generated.h"

class USavableObjectMarkerComponent;
class UBlackboardComponent;
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
	UPawnSensingComponent* PawnSensingComponent;
	UPROPERTY(EditAnywhere)
	UBlackboardComponent* ControllingBlackboard;
	
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;

	UFUNCTION()
	void OnSeePawn(APawn* Pawn); 
};
