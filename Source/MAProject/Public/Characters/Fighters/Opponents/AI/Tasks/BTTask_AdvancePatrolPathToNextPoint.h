// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_AdvancePatrolPathToNextPoint.generated.h"

/**
 * 
 */
UCLASS()
class MAPROJECT_API UBTTask_AdvancePatrolPathToNextPoint : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
public:
	UBTTask_AdvancePatrolPathToNextPoint();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
