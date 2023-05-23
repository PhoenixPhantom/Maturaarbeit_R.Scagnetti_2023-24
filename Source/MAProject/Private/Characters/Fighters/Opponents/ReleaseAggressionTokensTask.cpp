// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/ReleaseAggressionTokensTask.h"

#include "AIController.h"
#include "Characters/Fighters/Opponents/OpponentCharacter.h"
#include "Characters/Fighters/Opponents/OpponentController.h"

UReleaseAggressionTokensTask::UReleaseAggressionTokensTask(FObjectInitializer const& ObjectInitializer)
{
	NodeName = TEXT("Release Aggression Tokens");
}

EBTNodeResult::Type UReleaseAggressionTokensTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	CastChecked<AOpponentController>(OwnerComp.GetAIOwner())->ReleaseAggressionToken(FReleaseTokenKey());	
	return EBTNodeResult::Succeeded;
}
