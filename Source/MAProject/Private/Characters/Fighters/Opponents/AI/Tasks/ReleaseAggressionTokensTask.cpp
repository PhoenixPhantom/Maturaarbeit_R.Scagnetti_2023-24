// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/Tasks/ReleaseAggressionTokensTask.h"
#include "Characters/Fighters/Opponents/AI/OpponentController.h"

UReleaseAggressionTokensTask::UReleaseAggressionTokensTask(FObjectInitializer const& ObjectInitializer)
{
	NodeName = TEXT("Release Aggression Tokens");
}

EBTNodeResult::Type UReleaseAggressionTokensTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	CastChecked<AOpponentController>(OwnerComp.GetAIOwner())->ReleaseAggressionToken(FReleaseTokenKey());	
	return EBTNodeResult::Succeeded;
}
