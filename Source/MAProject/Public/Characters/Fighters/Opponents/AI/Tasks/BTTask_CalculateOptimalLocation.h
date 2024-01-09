// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "Utility/CombatManager.h"
#include "BTTask_CalculateOptimalLocation.generated.h"

/**
 * Calculate the best location for the owner to be oriented around the target player
 * this is done by executing the task and is works for both active and passive owners
 */
UCLASS()
class MAPROJECT_API UBTTask_CalculateTargetLocation : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
public:
	UBTTask_CalculateTargetLocation();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere)
	ECombatParticipantStatus StatusToGenerateLocationFor;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector LastCombatStatus;
};
