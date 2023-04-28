// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/OpponentController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"

AOpponentController::AOpponentController() : DefaultBehaviorTree(nullptr)
{
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));

	UAISenseConfig_Sight* SightConfig = CastChecked<UAISenseConfig_Sight>(PerceptionComponent->GetSenseConfig(UAISense::GetSenseID<UAISense_Sight>()));
	SightConfig->SightRadius = 2000;
	SightConfig->LoseSightRadius = 3000;
	SightConfig->PeripheralVisionAngleDegrees = 45;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->AutoSuccessRangeFromLastSeenLocation = 200;
	//PerceptionComponent->ConfigureSense(SightConfig);

	//Register OnPerceptionUpdated delegate
	PerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &AOpponentController::OnPerceptionUpdated);
}

float AOpponentController::GetFieldOfView() const
{
	return CastChecked<UAISenseConfig_Sight>(PerceptionComponent->GetSenseConfig(UAISense::GetSenseID<UAISense_Sight>()))
		->PeripheralVisionAngleDegrees * 2.f;
}

void AOpponentController::OnPossess(APawn* InPawn)
{
	RunBehaviorTree(DefaultBehaviorTree);
	Super::OnPossess(InPawn);
}

void AOpponentController::OnPerceptionUpdated(const TArray<AActor*>& DetectedPawns)
{
	FActorPerceptionBlueprintInfo PerceptionInfo;
	for(AActor* DetectedPawn : DetectedPawns)
	{
		PerceptionComponent->GetActorsPerception(DetectedPawn, PerceptionInfo);
		for(FAIStimulus Stimulus : PerceptionInfo.LastSensedStimuli)
		{
			if(!Stimulus.WasSuccessfullySensed()) continue;
			if(Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
			{
				Blackboard->SetValueAsBool("HasSensedPlayer", true);
				Blackboard->SetValueAsVector("TargetLocation", DetectedPawn->GetActorLocation());
			}
		}
	}
}
