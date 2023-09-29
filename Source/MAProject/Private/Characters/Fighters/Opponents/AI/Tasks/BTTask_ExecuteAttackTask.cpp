// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/Tasks/BTTask_ExecuteAttackTask.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Fighters/Opponents/OpponentCharacter.h"
#include "Characters/Fighters/Opponents/AI/OpponentController.h"
#include "Components/CapsuleComponent.h"

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
	if (!OwningCharacter->GetAcceptedInputs().bCanAttack) return EBTNodeResult::Failed;

	const ACharacter* TargetCharacter = OwningCharacter->GetCombatTarget();
	if (!IsValid(TargetCharacter))
	{
		return EBTNodeResult::Failed;
	}
	
	TArray<FAttackProperties> AvailableAttacks;
	OwningCharacter->GetAvailableAttacks(AvailableAttacks);


	const float DistanceFromTarget = FVector::Distance(OwningCharacter->GetActorLocation(), TargetCharacter->GetActorLocation()) -
		OwningCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius() - AOpponentController::MoveToDistanceMarginOfError;
	
	//Sort through all available attacks and remove those that cannot be executed
	TArray<TTuple<float, FAttackProperties>> AttacksInRange;
	float LowestScore = std::numeric_limits<float>::max();
	for (const FAttackProperties& Attack : AvailableAttacks)
	{
		if (Attack.MaximalMovementDistance < DistanceFromTarget ||
			Attack.GetIsOnCd())
		{
#if WITH_EDITORONLY_DATA
			if(OwningCharacter->GetIsDebugging())
			{
				if(Attack.GetIsOnCd()) continue;
				GLog->Log(OwningCharacter->GetActorNameOrLabel() + " attack left out because: " +
					FString::SanitizeFloat(Attack.MaximalMovementDistance) + " < " + FString::SanitizeFloat(DistanceFromTarget));
			}
#endif
			continue;
		}
		const float Priority = Attack.GetPriority(DistanceFromTarget);
		AttacksInRange.Add({Priority, Attack});
		if (Priority < LowestScore) LowestScore = Priority;
	}

	if(AttacksInRange.IsEmpty())
	{
#if WITH_EDITORONLY_DATA
		if(OwningCharacter->GetIsDebugging())
			GLog->Log(OwningCharacter->GetActorNameOrLabel() + " cannot attack since there are no valid attacks.");
#endif
		return EBTNodeResult::Failed;
	}

	//Assign a relative score to each attack
	double TotalScore = 0.f;
	for (TTuple<float, FAttackProperties>& Attack : AttacksInRange)
	{
		Attack.Key /= LowestScore;
		TotalScore += Attack.Key;
	}

	//Randomly chose a valid attack
	FAttackProperties ChosenAttack;
	//we use double in the assignment because we could be dealing with very small numbers and we still want high accuracy
	float RandomNumber = static_cast<double>(rand()) * TotalScore / static_cast<double>(RAND_MAX);
	for (const TTuple<float, FAttackProperties>& Attack : AttacksInRange)
	{
		RandomNumber -= Attack.Key;
		if (RandomNumber <= 0.f)
		{
			ChosenAttack = Attack.Value;
			break;
		}
	}

	FDelegateHandle Handle = OwningCharacter->OnInputLimitsResetDelegate().AddUObject(
		this, &UBTTask_ExecuteAttackTask::OnAttackFinished);
	if (OwningCharacter->ExecuteAttack(ChosenAttack)) return EBTNodeResult::InProgress;
	OwningCharacter->OnInputLimitsResetDelegate().Remove(Handle);
	return EBTNodeResult::Failed;
}

void UBTTask_ExecuteAttackTask::OnAttackFinished(bool IsLimitDurationOver, bool& HasBeenCleared)
{
	if (IsLimitDurationOver) FinishLatentTask(*BehaviorTreeComponent, EBTNodeResult::Succeeded);
	else
	{
		if(!IsValid(OwningCharacter)) return;
		if(!HasBeenCleared)
		{
			OwningCharacter->OnInputLimitsResetDelegate().Clear();
			HasBeenCleared = true;
		}
		OwningCharacter->OnInputLimitsResetDelegate().AddUObject(this, &UBTTask_ExecuteAttackTask::OnReactionFinished);
	}
}

void UBTTask_ExecuteAttackTask::OnReactionFinished(bool IsLimitDurationOver, bool& HasBeenCleared)
{
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle,
		[this]()
		{
			FinishLatentTask(*BehaviorTreeComponent, EBTNodeResult::Failed);
		}, 2.f, false);
}
