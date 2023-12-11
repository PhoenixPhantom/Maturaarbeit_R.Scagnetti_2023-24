// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/Tasks/BTTask_ExecuteAttackTask.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Fighters/Attacks/AttackTree/AttackNode.h"
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
	const ACharacter* TargetCharacter = OwningCharacter->GetCombatTarget();
	if (!IsValid(TargetCharacter))
	{
		//this suggests that the attacker is not actively engaged in combat (which makes no sense when it is attacking)
		checkNoEntry();
		OwningCharacter->ExecuteOnAggressionTokensReleased(FExecuteOnAggressionTokensReleasedKey());
		return EBTNodeResult::Failed;
	}
	
	check(IsValid(OwningCharacter));
	if (!OwningCharacter->GetAcceptedInputs().IsAllowedInput(EInputType::Attack))
	{
		//this suggests that the character was staggered (or killed) before even starting the attack
		//--> the result should be the same "token retention"
		//that occurs when the opponent is staggered while executing the attack
		OnAttackFinished(false);
		return EBTNodeResult::InProgress;
	}

	UAttackNode* RequestedNode = OwningCharacter->GetRequestedAttack();
	float Distance = FVector::Distance(OwningCharacter->GetActorLocation(), TargetCharacter->GetActorLocation());
	if(!IsValid(RequestedNode) || Distance > RequestedNode->GetAttackProperties().MaximalMovementDistance)
	{
		//tokens can be granted without setting an attack, if the receiver of the token still has a
		//better score than the other options
		//it is also possible that the player moved out of the original attack's MaximalMovementDistance
		
		//We have to select an attack here (which can be done randomly since every
		//attack has an overall value overall >= 0, so every choice is good)		
		RequestedNode = OwningCharacter->GetRandomValidAttackInRange();
	}
	if(!IsValid(RequestedNode))
	{
		//having been granted a token but still not being able to attack, the attacker retains it's tokens
		//until it can attack (not optimal but reduces movement noise)
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
	//at this point, the character might have been killed
	if(!IsValid(OwningCharacter)) return;
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
