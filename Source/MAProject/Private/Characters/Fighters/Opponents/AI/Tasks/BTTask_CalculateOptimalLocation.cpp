// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/Tasks/BTTask_CalculateOptimalLocation.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Fighters/Opponents/AI/OpponentController.h"

UBTTask_CalculateOptimalLocation::UBTTask_CalculateOptimalLocation()
{
	NodeName = "Calculate the optimal location";
}

EBTNodeResult::Type UBTTask_CalculateOptimalLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AOpponentController* OwningController = CastChecked<AOpponentController>(OwnerComp.GetAIOwner());
	
	FVector TargetLocation;
	if(!OwningController->GenerateTargetLocation(TargetLocation)) return EBTNodeResult::Failed;
	
	OwnerComp.GetBlackboardComponent()->SetValueAsVector(BlackboardKey.SelectedKeyName, TargetLocation);
	return EBTNodeResult::Succeeded;
}
