// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/Services/CheckIsInRangeService.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"


UCheckIsInRangeService::UCheckIsInRangeService()
{
	NodeName = "Check is in Range";
	bNotifyBecomeRelevant = true;
}

void UCheckIsInRangeService::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(BlackboardKey.SelectedKeyName));
	OwningCharacter = OwnerComp.GetAIOwner()->GetPawn();
	if(!IsValid(Target) || !IsValid(OwningCharacter))
	{
		checkNoEntry();
		bNotifyTick = false;
	}
}

void UCheckIsInRangeService::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	if(FVector::Distance(Target->GetActorLocation(), OwningCharacter->GetActorLocation()) <= MaximalDistance)
		OwnerComp.RequestBranchEvaluation(EBTNodeResult::Succeeded);
}
