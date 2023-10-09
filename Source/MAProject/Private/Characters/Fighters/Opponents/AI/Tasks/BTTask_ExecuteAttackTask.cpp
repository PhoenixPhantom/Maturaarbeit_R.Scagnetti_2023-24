// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/Tasks/BTTask_ExecuteAttackTask.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Fighters/Attacks/AttackTree/AttackTreeNode.h"
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

	const UAttackTreeNode* SourceNode = OwningCharacter->GetCharacterStats()->Attacks.GetCurrentNode(GetWorld());


	const float DistanceFromTarget = FVector::Distance(OwningCharacter->GetActorLocation(), TargetCharacter->GetActorLocation()) -
		OwningCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius() - AOpponentController::MoveToDistanceMarginOfError;
	
	//Sort through all available attacks and remove those that cannot be executed
	TArray<TTuple<UGenericGraphNode*, float>> AttacksInRange;
	double TotalScore = 0.f;
	for (UGenericGraphNode* ChildNode : SourceNode->ChildrenNodes)
	{
		const FAttackProperties& AttackProperties = CastChecked<UAttackTreeNode>(ChildNode)->GetAttackProperties();
		if (AttackProperties.MaximalMovementDistance <= DistanceFromTarget ||
			AttackProperties.GetIsOnCd())
		{
#if WITH_EDITORONLY_DATA
			if(OwningCharacter->GetIsDebugging())
			{
				if(AttackProperties.GetIsOnCd()) continue;
				GLog->Log(OwningCharacter->GetActorNameOrLabel() + " attack left out because: " +
					FString::SanitizeFloat(AttackProperties.MaximalMovementDistance) + " < " + FString::SanitizeFloat(DistanceFromTarget));
			}
#endif
			continue;
		}
		const float Priority = AttackProperties.GetPriority(DistanceFromTarget);
		AttacksInRange.Add({ChildNode, Priority});
		TotalScore += Priority;
	}

	if(AttacksInRange.IsEmpty())
	{
#if WITH_EDITORONLY_DATA
		if(OwningCharacter->GetIsDebugging())
			GLog->Log(OwningCharacter->GetActorNameOrLabel() + " cannot attack since there are no valid attacks.");
#endif
		return EBTNodeResult::Failed;
	}

	//Randomly chose a valid attack
	UAttackTreeNode* ChosenNode = nullptr;
	float RandomNumber = FMath::FRandRange(0.0, TotalScore);
	for (const TTuple<UGenericGraphNode*, float>& Attack : AttacksInRange)
	{
		RandomNumber -= Attack.Value;
		if (RandomNumber <= 0.f)
		{
			ChosenNode = CastChecked<UAttackTreeNode>(Attack.Key);
			break;
		}
	}

	FDelegateHandle Handle =
		OwningCharacter->OnInputLimitsResetDelegate().AddUObject(this, &UBTTask_ExecuteAttackTask::OnAttackFinished);
	
	if (OwningCharacter->ExecuteAttackFromNode(ChosenNode, FExecuteAttackKey()))
		return EBTNodeResult::InProgress;
	
	OwningCharacter->OnInputLimitsResetDelegate().Remove(Handle);
	return EBTNodeResult::Failed;
}

void UBTTask_ExecuteAttackTask::OnAttackFinished(bool IsLimitDurationOver)
{
	if (IsLimitDurationOver) FinishLatentTask(*BehaviorTreeComponent, EBTNodeResult::Succeeded);
	else
	{
		if(!IsValid(OwningCharacter)) return;
		OwningCharacter->OnInputLimitsResetDelegate().AddUObject(this, &UBTTask_ExecuteAttackTask::OnReactionFinished);
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
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle,
		[this]()
		{
			FinishLatentTask(*BehaviorTreeComponent, EBTNodeResult::Failed);
		}, 2.f, false);
}
