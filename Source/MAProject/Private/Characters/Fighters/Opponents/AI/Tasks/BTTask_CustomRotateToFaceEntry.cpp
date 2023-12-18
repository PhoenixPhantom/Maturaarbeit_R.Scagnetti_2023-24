// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/Tasks/BTTask_CustomRotateToFaceEntry.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Fighters/Opponents/OpponentCharacter.h"
#include "Utility/NonPlayerFunctionality/CharacterRotationManagerComponent.h"

UBTTask_CustomRotateToFaceEntry::UBTTask_CustomRotateToFaceEntry()
{
}

EBTNodeResult::Type UBTTask_CustomRotateToFaceEntry::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AOpponentCharacter* OwningController = CastChecked<AOpponentCharacter>(OwnerComp.GetAIOwner()->GetCharacter());
	const FVector& TargetLocation = OwnerComp.GetBlackboardComponent()->GetValueAsVector(BlackboardKey.SelectedKeyName);
	OwningController->GetCharacterRotationManager()->SetRotationMode(ECharacterRotationMode::OrientToTarget,
		false, nullptr, TargetLocation);
	return EBTNodeResult::Succeeded;
}
