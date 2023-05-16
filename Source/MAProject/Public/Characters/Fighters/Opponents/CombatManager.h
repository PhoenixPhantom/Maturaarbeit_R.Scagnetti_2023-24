// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CombatManager.generated.h"

class APlayerCharacter;
UCLASS()
class MAPROJECT_API ACombatManager : public AActor
{
	GENERATED_BODY()

public:
	ACombatManager();

	APlayerCharacter* GetPlayerCharacter() const;

protected:
	UPROPERTY()
	APlayerController* PlayerController;
	
	virtual void BeginPlay() override;
};
