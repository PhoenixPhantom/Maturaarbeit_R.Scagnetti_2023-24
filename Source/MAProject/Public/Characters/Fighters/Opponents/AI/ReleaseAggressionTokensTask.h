// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "ReleaseAggressionTokensTask.generated.h"

/**
 * 
 */
UCLASS()
class MAPROJECT_API UReleaseAggressionTokensTask : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UReleaseAggressionTokensTask(FObjectInitializer const& ObjectInitializer);

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
};
