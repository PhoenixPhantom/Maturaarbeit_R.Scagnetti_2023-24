// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_SetBlackboardKeyValue.generated.h"

/**
 * 
 */
UCLASS()
class MAPROJECT_API UBTTask_SetBlackboardKeyValue : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
public:
	UBTTask_SetBlackboardKeyValue();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere)
	bool NewBool;
	UPROPERTY(EditAnywhere)
	float NewFloat;
	UPROPERTY(EditAnywhere)
	int32 NewInt;
	UPROPERTY(EditAnywhere)
	uint8 NewEnum;
	UPROPERTY(EditAnywhere)
	FVector NewVector;
	UPROPERTY(EditAnywhere)
	UObject* NewObject;
};
