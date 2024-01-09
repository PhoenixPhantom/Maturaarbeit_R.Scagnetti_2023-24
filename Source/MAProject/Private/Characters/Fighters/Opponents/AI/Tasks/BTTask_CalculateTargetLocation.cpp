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

	//Generally, we don't want to use this node to generate the target location, as the target location already updates
	//automatically when the target moves far enough
	if(static_cast<ECombatParticipantStatus>(OwnerComp.GetBlackboardComponent()->GetValueAsEnum(
		LastCombatStatus.SelectedKeyName)) == StatusToGenerateLocationFor) return EBTNodeResult::Succeeded;

	//but if we changed the participation status, this is still required
	FVector TargetLocation;
	if(!OwningController->UpdateCombatLocation(TargetLocation, StatusToGenerateLocationFor, false)) return EBTNodeResult::Failed;
	OwnerComp.GetBlackboardComponent()->SetValueAsVector(BlackboardKey.SelectedKeyName, TargetLocation);
	OwnerComp.GetBlackboardComponent()->SetValueAsEnum(BlackboardKey.SelectedKeyName,
		static_cast<uint8>(StatusToGenerateLocationFor));
	return EBTNodeResult::Succeeded;
}
