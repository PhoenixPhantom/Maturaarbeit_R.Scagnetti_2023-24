// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "Navigation/PathFollowingComponent.h"
#include "OrientAroundTarget.generated.h"

/**
 * 
 */
UCLASS()
class MAPROJECT_API UOrientAroundTarget : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
public:
	UOrientAroundTarget();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
protected:
	FAIRequestID RequestId;
	UBehaviorTreeComponent* OwnerBehaviorTree;
	uint8* LocalNodeMemory;
	
	UPROPERTY(EditAnywhere, meta=(ToolTip="Wheather AI should be oriented as a aggressive or passive entity"))
	bool bOrientAggressively;

	UFUNCTION()
	void OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);
};
