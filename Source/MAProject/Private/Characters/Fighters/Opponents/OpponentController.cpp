// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/OpponentController.h"

#include "AI/NavigationSystemBase.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Fighters/Opponents/CombatManager.h"
#include "Characters/Fighters/Opponents/OpponentCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
#include "Utility/NonPlayerFunctionality/MovementTarget.h"

AOpponentController::AOpponentController() : CurrentTarget(nullptr), DefaultBehaviorTree(nullptr)
{
	PrimaryActorTick.bCanEverTick = false;
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));

	//Register OnPerceptionUpdated delegate
	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AOpponentController::OnTargetPerceptionUpdated);
	
}

FPathFollowingRequestResult AOpponentController::MoveTo(const FAIMoveRequest& MoveRequest, FNavPathSharedPtr* OutPath)
{
	if(!IsValid(MoveTarget)) return Super::MoveTo(MoveRequest, OutPath);
	FAIMoveRequest ResultingRequest;
	CurrentTarget = nullptr;
	if(!MoveRequest.IsMoveToActorRequest())
	{
		MoveTarget->SetMovementTargetLocation(MoveRequest.GetGoalLocation(), FSetMovementTargetKey());
		ResultingRequest.SetGoalActor(MoveTarget);		
	}
	else if(MoveRequest.GetGoalActor() != MoveTarget)
	{
		//CurrentTarget = MoveRequest.GetGoalActor();
		MoveTarget->SetTargetActor(MoveRequest.GetGoalActor(), FSetMovementTargetKey());
		ResultingRequest.SetGoalActor(MoveTarget);
	}
	else ResultingRequest.SetGoalActor(MoveRequest.GetGoalActor());

	//Normal copy doesn't work so we have to do this
	ResultingRequest.SetAcceptanceRadius(MoveRequest.GetAcceptanceRadius());
	ResultingRequest.SetCanStrafe(MoveRequest.CanStrafe());
	ResultingRequest.SetNavigationFilter(MoveRequest.GetNavigationFilter());
	ResultingRequest.SetUsePathfinding(MoveRequest.IsUsingPathfinding());
	ResultingRequest.SetUserData(MoveRequest.GetUserData());
	ResultingRequest.SetUserFlags(MoveRequest.GetUserFlags());
	ResultingRequest.SetAllowPartialPath(MoveRequest.IsUsingPartialPaths());
	ResultingRequest.SetProjectGoalLocation(MoveRequest.IsProjectingGoal());
	ResultingRequest.SetReachTestIncludesAgentRadius(MoveRequest.IsReachTestIncludingAgentRadius());
	ResultingRequest.SetReachTestIncludesGoalRadius(MoveRequest.IsReachTestIncludingGoalRadius());
	return Super::MoveTo(ResultingRequest, OutPath);
}

float AOpponentController::GetFieldOfView() const
{
	const UAISenseConfig_Sight* SightConfig = CastChecked<UAISenseConfig_Sight>(
		PerceptionComponent->GetSenseConfig(UAISense::GetSenseID<UAISense_Sight>()));
	return SightConfig->PeripheralVisionAngleDegrees * 2.f;
}

void AOpponentController::BeginPlay()
{
	Super::BeginPlay();
	//There can only be one combat manager at any given time to prevent logic problems
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACombatManager::StaticClass(), Actors);
	CombatManager = CastChecked<ACombatManager>(Actors[0]);
}

void AOpponentController::BeginDestroy()
{
	Super::BeginDestroy();
	CombatManager->UnregisterCombatParticipant(ControlledOpponent, FManageCombatParticipantsKey());
}

void AOpponentController::OnPossess(APawn* InPawn)
{
	RunBehaviorTree(DefaultBehaviorTree);
	Super::OnPossess(InPawn);
	ControlledOpponent = CastChecked<AOpponentCharacter>(InPawn);
	ControlledOpponent->SetLocalFieldOfView(GetFieldOfView(), FSetFieldOfViewKey());
	if(!IsValid(MoveTarget))
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParameters.Name = ToCStr(GetPawn()->GetName() + "_MovementTarget");
		MoveTarget = GetWorld()->SpawnActor<AMovementTarget>(AMovementTarget::StaticClass(), SpawnParameters);

	}
	MoveTarget->SetActorLocation(GetPawn()->GetActorLocation());
}

void AOpponentController::RegisterSensedPlayer(AActor* Player)
{
	Blackboard->SetValueAsBool("HasSensedPlayer", true);
	Blackboard->SetValueAsObject("TargetObject", Player);
	CombatManager->RegisterCombatParticipant(ControlledOpponent, FManageCombatParticipantsKey());
}

void AOpponentController::UnregisterSensedPlayer(AActor* Player)
{
	Blackboard->ClearValue("HasSensedPlayer");
	Blackboard->ClearValue("TargetObject");
	CombatManager->UnregisterCombatParticipant(ControlledOpponent, FManageCombatParticipantsKey());
}


void AOpponentController::OnTargetPerceptionUpdated(AActor* UpdatedActor, FAIStimulus Stimulus)
{
	if(!Stimulus.WasSuccessfullySensed())
	{
		if(!GetWorldTimerManager().TimerExists(LostPerceptionHandle)){  
			GetWorldTimerManager().SetTimer(LostPerceptionHandle, [this, UpdatedActor]
			{
				UnregisterSensedPlayer(UpdatedActor);
			}, 2.f, true);
		}
	}
	else
	{
		if(GetWorldTimerManager().TimerExists(LostPerceptionHandle)) GetWorldTimerManager().ClearTimer(LostPerceptionHandle);
		RegisterSensedPlayer(UpdatedActor);
	}
}

#if WITH_EDITORONLY_DATA
void AOpponentController::ToggleDebugging() const
{
	MoveTarget->SetIsDebugging(!MoveTarget->GetIsDebugging());
}
#endif
