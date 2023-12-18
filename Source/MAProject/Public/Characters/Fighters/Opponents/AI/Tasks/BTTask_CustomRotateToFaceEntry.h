// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_RotateToFaceBBEntry.h"
#include "BTTask_CustomRotateToFaceEntry.generated.h"

/**
 * 
 */
UCLASS()
class MAPROJECT_API UBTTask_CustomRotateToFaceEntry : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
public:
	UBTTask_CustomRotateToFaceEntry();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
