// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "RecalculateOrientationAroundTargetTask.generated.h"

class AOpponentCharacter;
class AOpponentController;

/**
 * Calculate the best location for the owner to be oriented around the target player
 * this is done by executing the task and is works for both active and passive owners
 */
UCLASS()
class MAPROJECT_API URecalculateOrientationAroundTargetTask : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
public:
	URecalculateOrientationAroundTargetTask();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
