// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/Tasks/BTTask_CustomMoveTo.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Fighters/FighterCharacter.h"

UBTTask_CustomMoveTo::UBTTask_CustomMoveTo(): ForcedMovementType(EForcedMovementType::PreferCurrent)
{
	NodeName = TEXT("Custom Move To");
}

EBTNodeResult::Type UBTTask_CustomMoveTo::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
                                                      uint8* NodeMemory)
{
	AFighterCharacter* Character =
		CastChecked<AFighterCharacter>(OwnerComp.GetAIOwner()->GetPawn());

	EBTNodeResult::Type NodeResult = EBTNodeResult::Failed;
	switch(ForcedMovementType)
	{
	case EForcedMovementType::PreferCurrent:
		NodeResult = ExecutePreferCurrent(Character, OwnerComp, NodeMemory);
		break;
	case EForcedMovementType::ForceKeepCurrent:
		NodeResult = ExecuteForceKeepCurrent(Character, OwnerComp, NodeMemory);
		break;
	case EForcedMovementType::PreferWalking:
		NodeResult = ExecutePreferWalking(Character, OwnerComp, NodeMemory);
		break;
	case EForcedMovementType::PreferRunning:
		NodeResult = ExecutePreferRunning(Character, OwnerComp, NodeMemory);
		break;
	case EForcedMovementType::ForceWalking:
		NodeResult = ExecuteForceWalking(Character, OwnerComp, NodeMemory);
		break;
	case EForcedMovementType::ForceRunning:
		NodeResult = ExecuteForceRunning(Character, OwnerComp, NodeMemory);
		break;
	default:
		checkNoEntry();
	}

	
	if(NodeResult == EBTNodeResult::Succeeded)
	{
		//For some reason the node sometimes returns "Succeeded" when the target is still far away.
		//Therefore we do a sanity check to confirm that "Succeeded" results are actually such where the
		//ai is close enough to it's target
		if(FVector::Distance(Character->GetActorLocation(),
			OwnerComp.GetBlackboardComponent()->GetValueAsVector(BlackboardKey.SelectedKeyName)) > AcceptableRadius)
		{
			return EBTNodeResult::Failed;
		}
	}
	return NodeResult;
}

EBTNodeResult::Type UBTTask_CustomMoveTo::ExecutePreferCurrent(AFighterCharacter* Character,
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if(Character->IsRunning())
	{
		if(Character->GetAcceptedInputs().bCanRun)
		{
			return Super::ExecuteTask(OwnerComp, NodeMemory);
		}
		if(Character->GetAcceptedInputs().MovementProperties.bCanWalk)
			Character->SwitchMovementToWalk(FSetWalkOrRunKey());
	}
	if(Character->IsWalking())
	{
		if(Character->GetAcceptedInputs().MovementProperties.bCanWalk)
		{
			return Super::ExecuteTask(OwnerComp, NodeMemory);
		}
		if(Character->GetAcceptedInputs().bCanRun) Character->SwitchMovementToRun(FSetWalkOrRunKey());
	}
	return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_CustomMoveTo::ExecuteForceKeepCurrent(AFighterCharacter* Character,
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if(Character->IsRunning() && Character->GetAcceptedInputs().bCanRun)
	{
		return Super::ExecuteTask(OwnerComp, NodeMemory);
	}
	if(Character->IsWalking() && Character->GetAcceptedInputs().MovementProperties.bCanWalk)
	{
		return Super::ExecuteTask(OwnerComp, NodeMemory);
	}
	
	return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_CustomMoveTo::ExecutePreferWalking(AFighterCharacter* Character,
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if(Character->GetAcceptedInputs().MovementProperties.bCanWalk)
	{
		Character->SwitchMovementToWalk(FSetWalkOrRunKey());
		return Super::ExecuteTask(OwnerComp, NodeMemory);
	}
	if(Character->GetAcceptedInputs().bCanRun)
	{
		Character->SwitchMovementToRun(FSetWalkOrRunKey());
		return Super::ExecuteTask(OwnerComp, NodeMemory);
	}
	
	return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_CustomMoveTo::ExecutePreferRunning(AFighterCharacter* Character,
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if(Character->GetAcceptedInputs().bCanRun)
	{
		Character->SwitchMovementToRun(FSetWalkOrRunKey());
		return Super::ExecuteTask(OwnerComp, NodeMemory);
	}
	if(Character->GetAcceptedInputs().MovementProperties.bCanWalk)
	{
		Character->SwitchMovementToWalk(FSetWalkOrRunKey());
		return Super::ExecuteTask(OwnerComp, NodeMemory);
	}
	
	return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_CustomMoveTo::ExecuteForceWalking(AFighterCharacter* Character,
                                                         UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if(Character->GetAcceptedInputs().MovementProperties.bCanWalk)
	{
		Character->SwitchMovementToWalk(FSetWalkOrRunKey());
		return Super::ExecuteTask(OwnerComp, NodeMemory);
	}
	
	return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_CustomMoveTo::ExecuteForceRunning(AFighterCharacter* Character,
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if(Character->GetAcceptedInputs().bCanRun)
	{
		Character->SwitchMovementToRun(FSetWalkOrRunKey());
		return Super::ExecuteTask(OwnerComp, NodeMemory);
	}
	
	return EBTNodeResult::Failed;
}
