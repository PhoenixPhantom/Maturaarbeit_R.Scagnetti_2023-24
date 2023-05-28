// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/Services/CheckIsInRangeService.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"


UCheckIsInRangeService::UCheckIsInRangeService() : Target(nullptr)
{
	NodeName = "Check is in Range";
	bNotifyBecomeRelevant = true;
	bNotifyTick = true;
}

void UCheckIsInRangeService::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	OwningCharacter = OwnerComp.GetAIOwner()->GetPawn();
	if(BlackboardKey.SelectedKeyType->IsChildOf(UBlackboardKeyType_Object::StaticClass()))
	{
		Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(BlackboardKey.SelectedKeyName));
		if(!IsValid(Target) || !IsValid(OwningCharacter))
		{
			checkNoEntry();
			bNotifyTick = false;
		}
	}
	else if(BlackboardKey.SelectedKeyType->IsChildOf(UBlackboardKeyType_Vector::StaticClass()))
	{
		TargetLocation = OwnerComp.GetBlackboardComponent()->GetValueAsVector(BlackboardKey.SelectedKeyName);
	}
	else
	{
		checkNoEntry();
		bNotifyTick = false;
	}
}

void UCheckIsInRangeService::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	if(IsValid(Target)) TargetLocation = Target->GetActorLocation();
	if(FVector::Distance(TargetLocation, OwningCharacter->GetActorLocation()) <= MaximalDistance)
		OwnerComp.RequestBranchEvaluation(EBTNodeResult::Succeeded);
}
