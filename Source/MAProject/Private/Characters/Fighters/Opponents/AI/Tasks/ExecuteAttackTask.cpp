// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/Tasks/ExecuteAttackTask.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Fighters/Opponents/OpponentCharacter.h"

UExecuteAttackTask::UExecuteAttackTask() : OwningCharacter(nullptr)
{
	NodeName = "Execute Attack";
	ForceInstancing(true);
	BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UExecuteAttackTask, BlackboardKey),
		AController::StaticClass());
}

EBTNodeResult::Type UExecuteAttackTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	BehaviorTreeComponent = &OwnerComp;
	if(OwningCharacter != OwnerComp.GetAIOwner()->GetPawn())
		OwningCharacter = CastChecked<AOpponentCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	if(!OwningCharacter->GetAcceptedInputs().bCanAttack) return EBTNodeResult::Failed;

	const AController* TargetController =
		Cast<AController>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(BlackboardKey.SelectedKeyName));
	if(!IsValid(TargetController)) return EBTNodeResult::Failed;

	const float Distance = FVector::Distance(TargetController->GetPawn()->GetActorLocation(),
		OwningCharacter->GetActorLocation());
	TArray<FAttackProperties> AvailableAttacks;
	OwningCharacter->GetAvailableAttacks(AvailableAttacks);

	//Sort through all available attacks and remove those that cannot be executed
	TArray<TTuple<float, FAttackProperties>> AttacksInRange;
	float LowestScore = std::numeric_limits<float>::max();
	for(const FAttackProperties& Attack : AvailableAttacks)
	{
		if(Attack.MaximalMovementDistance < Distance || Attack.GetIsOnCd()) continue;
		const float Priority = Attack.GetPriority(Distance);
		AttacksInRange.Add({Priority, Attack});
		if(Priority < LowestScore) LowestScore = Priority;
	}

	//Assign a relative score to each attack
	double TotalScore = 0.f;
	for(TTuple<float, FAttackProperties>& Attack : AttacksInRange)
	{
		Attack.Key /= LowestScore;
		TotalScore += Attack.Key;
	}

	//Randomly chose a valid attack
	FAttackProperties ChosenAttack;
	//we use double in the assignment because we could be dealing with very small numbers and we still want high accuracy
	float RandomNumber = static_cast<double>(rand()) * TotalScore / static_cast<double>(RAND_MAX);
	for(const TTuple<float, FAttackProperties>& Attack : AttacksInRange)
	{
		RandomNumber -= Attack.Key;
		if(RandomNumber <= 0.f)
		{
			ChosenAttack = Attack.Value;
			break;
		}
	}
	
	FDelegateHandle Handle = OwningCharacter->OnInputLimitsResetDelegate().AddUObject(this, &UExecuteAttackTask::OnAttackFinished);
	if(OwningCharacter->ExecuteAttack(ChosenAttack)) return EBTNodeResult::InProgress;
	OwningCharacter->OnInputLimitsResetDelegate().Remove(Handle);
	return EBTNodeResult::Failed;
}

void UExecuteAttackTask::OnAttackFinished(bool IsLimitDurationOver, bool& HasBeenCleared)
{
	if(IsLimitDurationOver) FinishLatentTask(*BehaviorTreeComponent, EBTNodeResult::Succeeded);
	else FinishLatentTask(*BehaviorTreeComponent, EBTNodeResult::Failed);
}
