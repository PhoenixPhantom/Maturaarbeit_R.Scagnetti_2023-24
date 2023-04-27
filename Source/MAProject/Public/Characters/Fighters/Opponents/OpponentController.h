// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "OpponentController.generated.h"

/**
 * 
 */
UCLASS()
class MAPROJECT_API AOpponentController : public AAIController
{
	GENERATED_BODY()
public:
	AOpponentController();
protected:
	UPROPERTY(EditAnywhere)
	UBehaviorTree* DefaultBehaviorTree;
	virtual void OnPossess(APawn* InPawn) override;

	
};
