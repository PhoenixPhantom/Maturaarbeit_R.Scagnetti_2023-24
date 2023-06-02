// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/Tasks/OrientAroundTarget.h"

#include "AIController.h"
#include "Characters/Fighters/Opponents/OpponentCharacter.h"
#include "Characters/Fighters/Opponents/AI/OpponentController.h"
#include "Utility/CombatManager.h"

UOrientAroundTarget::UOrientAroundTarget()
{
	NodeName = "Orient Around Target";
}

EBTNodeResult::Type UOrientAroundTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	OwnerBehaviorTree = &OwnerComp;
	LocalNodeMemory = NodeMemory;
	AOpponentController* OwningController = CastChecked<AOpponentController>(OwnerComp.GetAIOwner());
	AOpponentCharacter* OwningCharacter = CastChecked<AOpponentCharacter>(OwningController->GetPawn());
	OwningController->ReceiveMoveCompleted.AddDynamic(this, &UOrientAroundTarget::OnMoveCompleted);

	
	ACombatManager* CombatManager = OwningController->GetCombatManager();
	const FVector TargetLocation = CombatManager->GetAggressivenessDependantLocation(OwningCharacter);
	
	const FPathFollowingRequestResult RequestResult = OwningController->MoveTo(TargetLocation);
	RequestId = RequestResult.MoveId;
	if (RequestResult.Code == EPathFollowingRequestResult::AlreadyAtGoal) return EBTNodeResult::Succeeded;
	if(RequestResult.Code == EPathFollowingRequestResult::RequestSuccessful) return EBTNodeResult::InProgress;
	return EBTNodeResult::Failed;
}

void UOrientAroundTarget::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	if(RequestId != RequestID) return;
	const EBTNodeResult::Type TaskResult = ExecuteTask(*OwnerBehaviorTree, LocalNodeMemory);
	if(TaskResult == EBTNodeResult::InProgress) return;
	FinishLatentTask(*OwnerBehaviorTree, TaskResult);
}
