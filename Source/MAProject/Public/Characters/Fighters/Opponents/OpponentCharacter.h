// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Fighters/FighterCharacter.h"
#include "OpponentCharacter.generated.h"

class USavableObjectMarkerComponent;
/**
 * 
 */
UCLASS()
class MAPROJECT_API AOpponentCharacter : public AFighterCharacter
{
	GENERATED_BODY()
public:
	AOpponentCharacter();
protected:
	UPROPERTY(EditAnywhere)
	USavableObjectMarkerComponent* SavableObjectMarkerComponent;
	
};
