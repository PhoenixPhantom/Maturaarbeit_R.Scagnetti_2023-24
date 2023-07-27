// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_CheckIsInRange.generated.h"

/**
 * 
 */
UCLASS()
class MAPROJECT_API UBTService_CheckIsInRange : public UBTService_BlackboardBase
{
	GENERATED_BODY()
public:
	UBTService_CheckIsInRange();
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	FVector TargetLocation;
	
	UPROPERTY()
	AActor* Target;

	UPROPERTY()
	AActor* OwningCharacter;
	
	UPROPERTY(EditAnywhere)
	float MaximalDistance;
};
