// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/Tasks/BTTask_CustomMoveTo.h"

#include "AIController.h"
#include "Characters/Fighters/FighterCharacter.h"

EBTNodeResult::Type UBTTask_CustomMoveTo::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
                                                      uint8* NodeMemory)
{
	AFighterCharacter* Character =
		CastChecked<AFighterCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	if(Character->IsRunning())
	{
		if(Character->GetAcceptedInputs().bCanSprint)
		{
			return Super::ExecuteTask(OwnerComp, NodeMemory);
		}
		if(Character->GetAcceptedInputs().MovementProperties.bCanWalk) unimplemented();
	}
	if(Character->IsWalking())
	{
		if(Character->GetAcceptedInputs().MovementProperties.bCanWalk)
		{
			return Super::ExecuteTask(OwnerComp, NodeMemory);
		}
		if(Character->GetAcceptedInputs().bCanSprint) unimplemented();
	}
	return EBTNodeResult::Failed;
	
}
