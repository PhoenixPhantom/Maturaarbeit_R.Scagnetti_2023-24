// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/Tasks/BTTask_SetBlackboardKeyValue.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Int.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_NativeEnum.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

UBTTask_SetBlackboardKeyValue::UBTTask_SetBlackboardKeyValue(): NewBool(false), NewFloat(0), NewInt(0), NewEnum(0),
                                                                NewObject(nullptr)
{
	NodeName = "Set Blackboard Key Value";
}

EBTNodeResult::Type UBTTask_SetBlackboardKeyValue::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if(BlackboardKey.SelectedKeyType == UBlackboardKeyType_Bool::StaticClass())
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(BlackboardKey.SelectedKeyName, NewBool);
	}
	else if(BlackboardKey.SelectedKeyType == UBlackboardKeyType_Float::StaticClass())
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsFloat(BlackboardKey.SelectedKeyName, NewFloat);		
	}
	else if(BlackboardKey.SelectedKeyType == UBlackboardKeyType_Int::StaticClass())
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsInt(BlackboardKey.SelectedKeyName, NewInt);		
	}
	else if(BlackboardKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsVector(BlackboardKey.SelectedKeyName, NewVector);		
	}
	else if(BlackboardKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsObject(BlackboardKey.SelectedKeyName, NewObject);		
	}
	else if(BlackboardKey.SelectedKeyType == UBlackboardKeyType_NativeEnum::StaticClass())
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsEnum(BlackboardKey.SelectedKeyName, NewEnum);		
	}

	
	return EBTNodeResult::Succeeded;
}
