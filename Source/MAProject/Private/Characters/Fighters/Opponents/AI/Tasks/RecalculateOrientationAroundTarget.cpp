// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/Tasks/RecalculateOrientationAroundTarget.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Fighters/Opponents/OpponentCharacter.h"
#include "Characters/Fighters/Opponents/AI/OpponentController.h"
#include "Utility/CombatManager.h"

URecalculateOrientationAroundTarget::URecalculateOrientationAroundTarget() : OwningController(nullptr), OwningCharacter(nullptr)
{
	NodeName = "Recalculate Orientation Around Target";
}

EBTNodeResult::Type URecalculateOrientationAroundTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	
	if(!IsValid(OwningController)) OwningController = CastChecked<AOpponentController>(OwnerComp.GetAIOwner());
	if(!IsValid(OwningCharacter)) OwningCharacter = CastChecked<AOpponentCharacter>(OwningController->GetPawn());


	FVector TargetLocation;
	if(!OwningController->GetCombatManager()->
		GetAggressivenessDependantLocation(TargetLocation, OwningCharacter)) return EBTNodeResult::Failed;
	OwnerComp.GetBlackboardComponent()->SetValueAsVector(BlackboardKey.SelectedKeyName, TargetLocation);
	return EBTNodeResult::Succeeded;
}
