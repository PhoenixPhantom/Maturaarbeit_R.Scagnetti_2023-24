// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/Tasks/ReleaseAggressionTokensTask.h"

#include "AIController.h"
#include "Characters/Fighters/Opponents/OpponentCharacter.h"

UReleaseAggressionTokensTask::UReleaseAggressionTokensTask(FObjectInitializer const& ObjectInitializer)
{
	NodeName = TEXT("Release Aggression Tokens");
}

EBTNodeResult::Type UReleaseAggressionTokensTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	CastChecked<AOpponentCharacter>(OwnerComp.GetAIOwner()->GetPawn())->
		ExecuteOnAggressionTokensReleased(FExecuteOnAggressionTokensReleasedKey());	
	return EBTNodeResult::Succeeded;
}
