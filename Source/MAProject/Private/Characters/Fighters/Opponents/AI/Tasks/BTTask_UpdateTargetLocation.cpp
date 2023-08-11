// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/Tasks/BTTask_UpdateTargetLocation.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Fighters/Opponents/AI/OpponentController.h"

UBTTask_UpdateTargetLocation::UBTTask_UpdateTargetLocation()
{
	NodeName = "Update the target location if needed";
}

EBTNodeResult::Type UBTTask_UpdateTargetLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AOpponentController* OwningController = CastChecked<AOpponentController>(OwnerComp.GetAIOwner());
	
	//Only update target location if the current one isn't okay anymore (reduces movement noise)
	const FVector CurrentTarget = OwnerComp.GetBlackboardComponent()->GetValueAsVector(BlackboardKey.SelectedKeyName);
	if(OwningController->IsValidTargetLocation(CurrentTarget)) return EBTNodeResult::Succeeded;

	FVector TargetLocation;
	if(!OwningController->GenerateTargetLocation(TargetLocation)) return EBTNodeResult::Failed;
	
	OwnerComp.GetBlackboardComponent()->SetValueAsVector(BlackboardKey.SelectedKeyName, TargetLocation);
	return EBTNodeResult::Succeeded;
}
