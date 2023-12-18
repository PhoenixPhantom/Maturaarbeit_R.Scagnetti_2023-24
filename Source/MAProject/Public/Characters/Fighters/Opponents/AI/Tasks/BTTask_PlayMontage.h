// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_PlayMontage.generated.h"

/**
 * 
 */
UCLASS()
class MAPROJECT_API UBTTask_PlayMontage : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_PlayMontage();

protected:
	FTimerHandle TimerHandle;
	UPROPERTY(Category = Node, EditAnywhere)
	UAnimMontage* AnimationToPlay;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
