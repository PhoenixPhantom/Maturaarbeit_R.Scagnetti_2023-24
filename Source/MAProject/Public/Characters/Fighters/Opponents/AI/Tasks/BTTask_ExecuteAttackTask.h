// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_ExecuteAttackTask.generated.h"

class AOpponentCharacter;

/**
 * 
 */
UCLASS()
class MAPROJECT_API UBTTask_ExecuteAttackTask : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
public:
	UBTTask_ExecuteAttackTask();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
protected:

	UPROPERTY()
	UBehaviorTreeComponent* BehaviorTreeComponent;
	
	UPROPERTY()
	AOpponentCharacter* OwningCharacter;

	UFUNCTION()
	void OnAttackFinished(bool IsLimitDurationOver);

	UFUNCTION()
	void OnReactionFinished(bool IsLimitDurationOver);
};
