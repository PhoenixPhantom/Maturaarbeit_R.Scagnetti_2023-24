// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/Tasks/BTTask_ReleaseAggressionTokens.h"

#include "AIController.h"
#include "Characters/Fighters/Opponents/OpponentCharacter.h"

UBTTask_ReleaseAggressionTokens::UBTTask_ReleaseAggressionTokens(FObjectInitializer const& ObjectInitializer)
{
	NodeName = TEXT("Release Aggression Tokens");
}

EBTNodeResult::Type UBTTask_ReleaseAggressionTokens::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	CastChecked<AOpponentCharacter>(OwnerComp.GetAIOwner()->GetPawn())->
		ExecuteOnAggressionTokensReleased(FExecuteOnAggressionTokensReleasedKey());	
	return EBTNodeResult::Succeeded;
}
