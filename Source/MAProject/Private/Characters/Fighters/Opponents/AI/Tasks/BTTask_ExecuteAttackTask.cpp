// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/Tasks/BTTask_ExecuteAttackTask.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Fighters/Attacks/AttackTree/AttackTreeNode.h"
#include "Characters/Fighters/Opponents/OpponentCharacter.h"

UBTTask_ExecuteAttackTask::UBTTask_ExecuteAttackTask() : OwningCharacter(nullptr)
{
	NodeName = "Execute Attack";
	ForceInstancing(true);
	BlackboardKey.AllowNoneAsValue(true);
}

EBTNodeResult::Type UBTTask_ExecuteAttackTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	BehaviorTreeComponent = &OwnerComp;
	OwningCharacter = CastChecked<AOpponentCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	check(IsValid(OwningCharacter));
	if (!OwningCharacter->GetAcceptedInputs().bCanAttack)
	{
		checkNoEntry();
		OwningCharacter->ExecuteOnAggressionTokensReleased(FExecuteOnAggressionTokensReleasedKey());
		return EBTNodeResult::Failed;
	}

	const ACharacter* TargetCharacter = OwningCharacter->GetCombatTarget();
	if (!IsValid(TargetCharacter))
	{
		checkNoEntry();
		OwningCharacter->ExecuteOnAggressionTokensReleased(FExecuteOnAggressionTokensReleasedKey());
		return EBTNodeResult::Failed;
	}

	UAttackTreeNode* RequestedNode = OwningCharacter->GetRequestedAttack();
	if(!IsValid(RequestedNode))
	{
		RequestedNode = OwningCharacter->GetRandomValidAttackInRange();
	}
	if(!IsValid(RequestedNode))
	{
#if WITH_EDITORONLY_DATA
		if(OwningCharacter->GetIsDebugging())
			GLog->Log(OwningCharacter->GetActorNameOrLabel() + " cannot attack since there are no valid attacks.");
#endif
		return EBTNodeResult::Failed;
	}

	TDelegate<void(bool)> OnAttackFinished;
	OnAttackFinished.BindUObject(this, &UBTTask_ExecuteAttackTask::OnAttackFinished);
	OwningCharacter->AddOnInputLimitsResetDelegate(OnAttackFinished, FModifyInputLimitsKey());
	
	if (OwningCharacter->ExecuteAttackFromNode(RequestedNode, FExecuteAttackKey()))
	{
		OwningCharacter->ClearRequestedAttack(FClearRequestedAttackKey());
		return EBTNodeResult::InProgress;
	}
	
	OwningCharacter->RemoveOnInputLimitsResetDelegate(OnAttackFinished, FModifyInputLimitsKey());
	return EBTNodeResult::Failed;
}

void UBTTask_ExecuteAttackTask::OnAttackFinished(bool IsLimitDurationOver)
{
	if (IsLimitDurationOver)
	{
		OwningCharacter->ExecuteOnAggressionTokensReleased(FExecuteOnAggressionTokensReleasedKey());
		FinishLatentTask(*BehaviorTreeComponent, EBTNodeResult::Succeeded);
	}
	else
	{
		if(!IsValid(OwningCharacter)) return;
		TDelegate<void(bool)> OnInterruptionEnded;
		OnInterruptionEnded.BindUObject(this, &UBTTask_ExecuteAttackTask::OnReactionFinished);
		OwningCharacter->AddOnInputLimitsResetDelegate(OnInterruptionEnded, FModifyInputLimitsKey());
	}
}

void UBTTask_ExecuteAttackTask::OnReactionFinished(bool IsLimitDurationOver)
{
#if WITH_EDITORONLY_DATA
	if(OwningCharacter->GetIsDebugging())
	{
		GLog->Log("The attack was interrupted...");
	}
#endif

	//after an attack being interrupted, the opponent will retain it's aggression tokens for a short time, to give the player a "reward"
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle,
		[this]()
		{
			OwningCharacter->ExecuteOnAggressionTokensReleased(FExecuteOnAggressionTokensReleasedKey());
			FinishLatentTask(*BehaviorTreeComponent, EBTNodeResult::Failed);
		}, 2.f, false);
}
