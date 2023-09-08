// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/Tasks/BTTask_AdvancePatrolPathToNextPoint.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Utility/Navigation/PatrolManagerComponent.h"

UBTTask_AdvancePatrolPathToNextPoint::UBTTask_AdvancePatrolPathToNextPoint()
{
	NodeName = TEXT("Advance Patrol Path To Next Point");
	BlackboardKey.AllowNoneAsValue(true);
}

EBTNodeResult::Type UBTTask_AdvancePatrolPathToNextPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	UPatrolManagerComponent* PatrolManager =
		Cast<UPatrolManagerComponent>(OwnerComp.GetAIOwner()->GetCharacter()->GetComponentByClass(
			UPatrolManagerComponent::StaticClass()));

	if(!IsValid(PatrolManager))
	{
		checkNoEntry();
		return EBTNodeResult::Failed;
	}
	
	
	PatrolManager->AdvanceToNextPathPoint();
	return EBTNodeResult::Succeeded;
}
