// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/Tasks/BTTask_CalculateOptimalLocation.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Fighters/Opponents/AI/OpponentController.h"

UBTTask_CalculateTargetLocation::UBTTask_CalculateTargetLocation()
{
	NodeName = "Calculate the desired target location";
	BlackboardKey.AddVectorFilter(this,
		GET_MEMBER_NAME_CHECKED(UBTTask_CalculateTargetLocation, BlackboardKey));
}

EBTNodeResult::Type UBTTask_CalculateTargetLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AOpponentController* OwningController = CastChecked<AOpponentController>(OwnerComp.GetAIOwner());
	
	FVector TargetLocation;
	if(!OwningController->GenerateCombatLocation(TargetLocation, StatusToGenerateLocationFor)) return EBTNodeResult::Failed;
	OwnerComp.GetBlackboardComponent()->SetValueAsVector(BlackboardKey.SelectedKeyName, TargetLocation);
	return EBTNodeResult::Succeeded;
}
