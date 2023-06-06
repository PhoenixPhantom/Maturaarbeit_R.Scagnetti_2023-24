// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "RecalculateOrientationAroundTarget.generated.h"

class AOpponentCharacter;
class AOpponentController;

/**
 * Calculate the best location for the owner to be oriented around the target player
 * this is done by executing the task and is works for both active and passive owners
 */
UCLASS()
class MAPROJECT_API URecalculateOrientationAroundTarget : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
public:
	URecalculateOrientationAroundTarget();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
protected:
	
	UPROPERTY()
	AOpponentController* OwningController;
	
	UPROPERTY()
	AOpponentCharacter* OwningCharacter;
};
