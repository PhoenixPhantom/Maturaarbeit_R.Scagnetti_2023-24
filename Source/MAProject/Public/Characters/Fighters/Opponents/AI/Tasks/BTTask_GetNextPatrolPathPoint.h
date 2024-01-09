// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_GetNextPatrolPathPoint.generated.h"

/**
 * 
 */
UCLASS()
class MAPROJECT_API UBTTask_GetNextPatrolPathPoint : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
public:
	UBTTask_GetNextPatrolPathPoint();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector StartFromClosestPathPointKey;
};
