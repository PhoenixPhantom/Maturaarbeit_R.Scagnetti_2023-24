// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/Tasks/RecalculateOrientationAroundTargetTask.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Fighters/Opponents/OpponentCharacter.h"
#include "Characters/Fighters/Opponents/AI/OpponentController.h"
#include "Utility/CombatManager.h"

URecalculateOrientationAroundTargetTask::URecalculateOrientationAroundTargetTask()
{
	NodeName = "Recalculate Orientation Around Target";
}

EBTNodeResult::Type URecalculateOrientationAroundTargetTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AOpponentController* OwningController = CastChecked<AOpponentController>(OwnerComp.GetAIOwner());
	AOpponentCharacter* OwningCharacter = CastChecked<AOpponentCharacter>(OwningController->GetPawn());
	
	FVector TargetLocation;
	if(!OwningController->GetOptimalLocation(TargetLocation)) return EBTNodeResult::Failed;
	
	OwnerComp.GetBlackboardComponent()->SetValueAsVector(BlackboardKey.SelectedKeyName, TargetLocation);
	return EBTNodeResult::Succeeded;
}
