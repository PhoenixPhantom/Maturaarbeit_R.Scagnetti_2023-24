// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_RotateToFaceBBEntry.generated.h"

/**
 * 
 */
UCLASS()
class MAPROJECT_API UBTService_RotateToFaceBBEntry : public UBTService_BlackboardBase
{
	GENERATED_BODY()
public:
	UBTService_RotateToFaceBBEntry();
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	FVector TargetLocation;
	
	UPROPERTY()
	AActor* TargetObject;

	UPROPERTY()
	AController* TargetController;

	UPROPERTY()
	AAIController* OwningController;
};
