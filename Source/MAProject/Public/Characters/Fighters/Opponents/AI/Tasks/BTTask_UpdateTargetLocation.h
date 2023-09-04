// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_UpdateTargetLocation.generated.h"

/**
 * 
 */
UCLASS()
class MAPROJECT_API UBTTask_UpdateTargetLocation : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
public:
	UBTTask_UpdateTargetLocation();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	float LastTimestamp;

	UPROPERTY(EditAnywhere)
	float MinDelayTime;

	UPROPERTY(EditAnywhere)
	float MaxDelayTime;
};
