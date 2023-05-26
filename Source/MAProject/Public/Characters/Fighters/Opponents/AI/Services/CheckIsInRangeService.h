// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "CheckIsInRangeService.generated.h"

/**
 * 
 */
UCLASS()
class MAPROJECT_API UCheckIsInRangeService : public UBTService_BlackboardBase
{
	GENERATED_BODY()
public:
	UCheckIsInRangeService();
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	UPROPERTY()
	AActor* Target;

	UPROPERTY()
	AActor* OwningCharacter;
	
	UPROPERTY(EditAnywhere)
	float MaximalDistance;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector AbortConditionKey;
};
