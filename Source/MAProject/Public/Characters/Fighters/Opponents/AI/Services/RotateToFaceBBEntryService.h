// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "RotateToFaceBBEntryService.generated.h"

/**
 * 
 */
UCLASS()
class MAPROJECT_API URotateToFaceBBEntryService : public UBTService_BlackboardBase
{
	GENERATED_BODY()
public:
	URotateToFaceBBEntryService();
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	FVector TargetLocation;
	
	UPROPERTY()
	AActor* TargetObject;

	UPROPERTY()
	AAIController* OwningController;
};
