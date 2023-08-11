// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/Tasks/BTTask_CalculateOptimalLocation.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Fighters/Opponents/AI/OpponentController.h"

UBTTask_CalculateTargetLocation::UBTTask_CalculateTargetLocation()
{
	NodeName = "Calculate the desired target location";
}

EBTNodeResult::Type UBTTask_CalculateTargetLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AOpponentController* OwningController = CastChecked<AOpponentController>(OwnerComp.GetAIOwner());

	//No need to move when staying also works (reduces movement noise)
	//if(OwningController->IsValidTargetLocation(OwningController->GetPawn()->GetActorLocation()))
	//	return EBTNodeResult::Succeeded;
	
	FVector TargetLocation;
	if(!OwningController->GenerateTargetLocation(TargetLocation)) return EBTNodeResult::Failed;
	OwnerComp.GetBlackboardComponent()->SetValueAsVector(BlackboardKey.SelectedKeyName, TargetLocation);
	return EBTNodeResult::Succeeded;
}
