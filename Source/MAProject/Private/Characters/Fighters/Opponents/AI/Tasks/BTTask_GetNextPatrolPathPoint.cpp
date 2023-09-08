// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/Tasks/BTTask_GetNextPatrolPathPoint.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Utility/Navigation/PatrolManagerComponent.h"

UBTTask_GetNextPatrolPathPoint::UBTTask_GetNextPatrolPathPoint()
{
	NodeName = TEXT("Get Next Patrol Path Point");
	
	BlackboardKey.AddVectorFilter(this,
		GET_MEMBER_NAME_CHECKED(UBTTask_GetNextPatrolPathPoint, BlackboardKey));
}

EBTNodeResult::Type UBTTask_GetNextPatrolPathPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UPatrolManagerComponent* PatrolManager =
		Cast<UPatrolManagerComponent>(OwnerComp.GetAIOwner()->GetCharacter()->GetComponentByClass(
			UPatrolManagerComponent::StaticClass()));

	if(!IsValid(PatrolManager))
	{
		checkNoEntry();
		return EBTNodeResult::Failed;
	}
	
	const FVector NextPathPointLocation = PatrolManager->GetNextPathPointLocation();
	if(NextPathPointLocation.ContainsNaN()) return EBTNodeResult::Failed;
	
	OwnerComp.GetBlackboardComponent()->SetValueAsVector(BlackboardKey.SelectedKeyName, NextPathPointLocation);
	return EBTNodeResult::Succeeded;
}
