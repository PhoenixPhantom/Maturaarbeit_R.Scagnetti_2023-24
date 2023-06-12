// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/Services/RotateToFaceBBEntryService.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

URotateToFaceBBEntryService::URotateToFaceBBEntryService() : TargetObject(nullptr), OwningController(nullptr)
{
	NodeName = "Rotate To Face BB Entry";
	bNotifyBecomeRelevant = true;
	bNotifyTick = true;
}

void URotateToFaceBBEntryService::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	OwningController = OwnerComp.GetAIOwner();
	if(BlackboardKey.SelectedKeyType->IsChildOf(UBlackboardKeyType_Object::StaticClass()))
	{
		TargetObject = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(BlackboardKey.SelectedKeyName));
		if(!IsValid(TargetObject) || !IsValid(OwningController))
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

void URotateToFaceBBEntryService::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	if(IsValid(TargetObject)) OwningController->SetFocus(TargetObject);
	else OwningController->SetFocalPoint(TargetLocation);
}
