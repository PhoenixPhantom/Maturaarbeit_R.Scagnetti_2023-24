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
	StartFromClosestPathPointKey.AddBoolFilter(this,
		GET_MEMBER_NAME_CHECKED(UBTTask_GetNextPatrolPathPoint, StartFromClosestPathPointKey));
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

	const bool RestartFromClosest = OwnerComp.GetBlackboardComponent()->GetValueAsBool(StartFromClosestPathPointKey.SelectedKeyName);
	
	const FVector NextPathPointLocation = PatrolManager->GetNextPathPointLocation(RestartFromClosest);
	
	//this bool works like a marker flag, so remove it once it has been acted upon
	if(RestartFromClosest) OwnerComp.GetBlackboardComponent()->SetValueAsBool(StartFromClosestPathPointKey.SelectedKeyName, false);
	
	if(NextPathPointLocation.ContainsNaN())
	{
		return EBTNodeResult::Failed;
	}
	
	OwnerComp.GetBlackboardComponent()->SetValueAsVector(BlackboardKey.SelectedKeyName, NextPathPointLocation);
	return EBTNodeResult::Succeeded;
}
