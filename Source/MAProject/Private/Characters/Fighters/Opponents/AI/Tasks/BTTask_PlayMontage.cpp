// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/Tasks/BTTask_PlayMontage.h"

#include "AIController.h"
#include "GameFramework/Character.h"

UBTTask_PlayMontage::UBTTask_PlayMontage(): AnimationToPlay(nullptr)
{
	NodeName = "Play Montage";
	// instantiating to be able to use Timers
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_PlayMontage::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	
	AAIController* const Controller = OwnerComp.GetAIOwner();
	if (!IsValid(AnimationToPlay) || !IsValid(Controller) || !IsValid(Controller->GetCharacter())) return EBTNodeResult::Failed;

	const float MontageDuration = Controller->GetCharacter()->PlayAnimMontage(AnimationToPlay);

	Controller->GetWorld()->GetTimerManager().SetTimer(TimerHandle, [Local = this, &OwnerComp]
	{
		//if(!IsValid(Local)) return;
		Local->FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		
	}, MontageDuration, false);

	return MontageDuration > 0.f ? EBTNodeResult::InProgress : EBTNodeResult::Failed;
}
