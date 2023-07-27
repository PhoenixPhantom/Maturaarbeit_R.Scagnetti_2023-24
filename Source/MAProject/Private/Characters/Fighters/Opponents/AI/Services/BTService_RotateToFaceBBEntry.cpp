// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/Services/BTService_RotateToFaceBBEntry.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

UBTService_RotateToFaceBBEntry::UBTService_RotateToFaceBBEntry() : TargetObject(nullptr), OwningController(nullptr)
{
	NodeName = "Rotate To Face BB Entry";
	bNotifyBecomeRelevant = true;
	bNotifyTick = true;
}

void UBTService_RotateToFaceBBEntry::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	OwningController = OwnerComp.GetAIOwner();
	if(!IsValid(OwningController))
	{
		checkNoEntry();
		bNotifyTick = false;
	}
	
	if(BlackboardKey.SelectedKeyType->IsChildOf(UBlackboardKeyType_Object::StaticClass()))
	{
		UObject* Object = OwnerComp.GetBlackboardComponent()->GetValueAsObject(BlackboardKey.SelectedKeyName);
		TargetController = Cast<AController>(Object);
		if(!IsValid(TargetController))
		{
			TargetController = nullptr;
			bNotifyTick = false;
		}
		else return;
		
		TargetObject = Cast<AActor>(Object);
		if(!IsValid(TargetObject))
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

void UBTService_RotateToFaceBBEntry::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	if(IsValid(TargetController)) OwningController->SetFocus(TargetController->GetPawn());
	else if(IsValid(TargetObject)) OwningController->SetFocus(TargetObject);
	else OwningController->SetFocalPoint(TargetLocation);
}
