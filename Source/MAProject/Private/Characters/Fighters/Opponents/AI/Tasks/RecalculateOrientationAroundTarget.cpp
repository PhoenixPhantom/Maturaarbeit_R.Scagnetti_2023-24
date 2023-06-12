// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/Tasks/RecalculateOrientationAroundTarget.h"

#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Fighters/Opponents/OpponentCharacter.h"
#include "Characters/Fighters/Opponents/AI/OpponentController.h"
#include "Utility/CombatManager.h"

URecalculateOrientationAroundTarget::URecalculateOrientationAroundTarget() : OwningController(nullptr),
	OwningCharacter(nullptr), AllowedDeltaDistance(50.f)
{
	NodeName = "Recalculate Orientation Around Target";
}

EBTNodeResult::Type URecalculateOrientationAroundTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	
	if(OwningController != OwnerComp.GetAIOwner()) OwningController = CastChecked<AOpponentController>(OwnerComp.GetAIOwner());
	if(OwningCharacter != OwningController->GetPawn()) OwningCharacter = CastChecked<AOpponentCharacter>(OwningController->GetPawn());


	FVector TargetLocation;
	if(!OwningController->GetCombatManager()->
		GetAggressivenessDependantLocation(TargetLocation, OwningCharacter)) return EBTNodeResult::Failed;
	
	if(AllowedDeltaDistance >= 0.f)
	{
		double PathLength = 0.0;
		UNavigationSystemV1::GetPathLength(GetWorld(), OwningCharacter->GetActorLocation(), TargetLocation,
			PathLength);
		if(PathLength <= AllowedDeltaDistance) TargetLocation = OwningCharacter->GetActorLocation();
	}
	
	OwnerComp.GetBlackboardComponent()->SetValueAsVector(BlackboardKey.SelectedKeyName, TargetLocation);
	return EBTNodeResult::Succeeded;
}
