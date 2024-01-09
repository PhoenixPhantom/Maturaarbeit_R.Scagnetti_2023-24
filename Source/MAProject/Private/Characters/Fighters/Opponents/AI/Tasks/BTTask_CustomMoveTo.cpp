// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/Tasks/BTTask_CustomMoveTo.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "Characters/Fighters/FighterCharacter.h"
#include "Characters/Fighters/Opponents/OpponentCharacter.h"
#include "Characters/Fighters/Opponents/AI/OpponentController.h"
#include "Components/CapsuleComponent.h"

UBTTask_CustomMoveTo::UBTTask_CustomMoveTo(): ForcedMovementType(EForcedMovementType::PreferCurrent)
{
	NodeName = TEXT("Custom Move To");
}

EBTNodeResult::Type UBTTask_CustomMoveTo::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
                                                      uint8* NodeMemory)
{
	AFighterCharacter* Character =
		CastChecked<AFighterCharacter>(OwnerComp.GetAIOwner()->GetCharacter());

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

		float RequiredRadius = AcceptableRadius + Character->GetCapsuleComponent()->GetScaledCapsuleRadius() +
			AOpponentCharacter::MoveToDistanceMarginOfError;
		if(BlackboardKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
		{
			const ACharacter* TargetCharacter =
				CastChecked<ACharacter>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(BlackboardKey.SelectedKeyName));
			RequiredRadius += TargetCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
		}
		
		if(FVector::Distance(Character->GetActorLocation(),
			OwnerComp.GetBlackboardComponent()->GetValueAsVector(BlackboardKey.SelectedKeyName)) > RequiredRadius)
		{
#if WITH_EDITORONLY_DATA
			if(Character->GetIsDebugging())
			{
				GLog->Log(Character->GetActorNameOrLabel() + " move to stopped at: " + FString::SanitizeFloat(FVector::Distance(Character->GetActorLocation(),
				OwnerComp.GetBlackboardComponent()->GetValueAsVector(BlackboardKey.SelectedKeyName))) + " > " + FString::SanitizeFloat(RequiredRadius));
			}
#endif
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
		if(Character->GetAcceptedInputs().IsAllowedInput(EInputType::Sprint))
		{
			return Super::ExecuteTask(OwnerComp, NodeMemory);
		}
		if(Character->GetAcceptedInputs().IsAllowedInput(EInputType::Walk))
			Character->SwitchMovementToWalk(FSetWalkOrRunKey());
	}
	if(Character->IsWalking())
	{
		if(Character->GetAcceptedInputs().IsAllowedInput(EInputType::Walk))
		{
			return Super::ExecuteTask(OwnerComp, NodeMemory);
		}
		if(Character->GetAcceptedInputs().IsAllowedInput(EInputType::Sprint)) Character->SwitchMovementToRun(FSetWalkOrRunKey());
	}
	return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_CustomMoveTo::ExecuteForceKeepCurrent(AFighterCharacter* Character,
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if(Character->IsRunning() && Character->GetAcceptedInputs().IsAllowedInput(EInputType::Sprint))
	{
		return Super::ExecuteTask(OwnerComp, NodeMemory);
	}
	if(Character->IsWalking() && Character->GetAcceptedInputs().IsAllowedInput(EInputType::Walk))
	{
		return Super::ExecuteTask(OwnerComp, NodeMemory);
	}
	
	return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_CustomMoveTo::ExecutePreferWalking(AFighterCharacter* Character,
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if(Character->GetAcceptedInputs().IsAllowedInput(EInputType::Walk))
	{
		Character->SwitchMovementToWalk(FSetWalkOrRunKey());
		return Super::ExecuteTask(OwnerComp, NodeMemory);
	}
	if(Character->GetAcceptedInputs().IsAllowedInput(EInputType::Sprint))
	{
		Character->SwitchMovementToRun(FSetWalkOrRunKey());
		return Super::ExecuteTask(OwnerComp, NodeMemory);
	}

	return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_CustomMoveTo::ExecutePreferRunning(AFighterCharacter* Character,
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if(Character->GetAcceptedInputs().IsAllowedInput(EInputType::Sprint))
	{
		Character->SwitchMovementToRun(FSetWalkOrRunKey());
		return Super::ExecuteTask(OwnerComp, NodeMemory);
	}
	if(Character->GetAcceptedInputs().IsAllowedInput(EInputType::Walk))
	{
		Character->SwitchMovementToWalk(FSetWalkOrRunKey());
		return Super::ExecuteTask(OwnerComp, NodeMemory);
	}
	
	return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_CustomMoveTo::ExecuteForceWalking(AFighterCharacter* Character,
                                                         UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if(Character->GetAcceptedInputs().IsAllowedInput(EInputType::Walk))
	{
		Character->SwitchMovementToWalk(FSetWalkOrRunKey());
		return Super::ExecuteTask(OwnerComp, NodeMemory);
	}
	
	return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_CustomMoveTo::ExecuteForceRunning(AFighterCharacter* Character,
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if(Character->GetAcceptedInputs().IsAllowedInput(EInputType::Sprint))
	{
		Character->SwitchMovementToRun(FSetWalkOrRunKey());
		return Super::ExecuteTask(OwnerComp, NodeMemory);
	}
	
	return EBTNodeResult::Failed;
}
