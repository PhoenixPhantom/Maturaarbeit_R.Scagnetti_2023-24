// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/OpponentController.h"

#include "AI/NavigationSystemBase.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Utility/CombatManager.h"
#include "Characters/Fighters/Opponents/OpponentCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
#include "Utility/NonPlayerFunctionality/MovementTarget.h"

void FAIMoveRequestExpanded::ForceSetGoalActor(const AActor* InGoalActor)
{
	GoalActor = const_cast<AActor*>(InGoalActor);
	GoalLocation = FAISystem::InvalidLocation;
	bMoveToActor = true;
	bInitialized = true;
}

void FAIMoveRequestExpanded::ForceSetGoalLocation(const FVector& InGoalLocation)
{
	GoalActor = nullptr;
	GoalLocation = InGoalLocation;
	bMoveToActor = false;
	bInitialized = true;
}

AOpponentController::AOpponentController() : DefaultBehaviorTree(nullptr)
{
	PrimaryActorTick.bCanEverTick = false;
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));

	//Register OnPerceptionUpdated delegate
	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AOpponentController::OnTargetPerceptionUpdated);
	
}

FPathFollowingRequestResult AOpponentController::MoveTo(const FAIMoveRequest& MoveRequest, FNavPathSharedPtr* OutPath)
{
	if(!IsValid(MoveTarget)) return Super::MoveTo(MoveRequest, OutPath);
	FAIMoveRequestExpanded ModifiedRequest = MoveRequest;
	if(!MoveRequest.IsMoveToActorRequest())
	{
		MoveTarget->SetMovementTargetLocation(MoveRequest.GetGoalLocation(), FSetMovementTargetKey());
		ModifiedRequest.ForceSetGoalActor(MoveTarget);
	}
	else if(MoveRequest.GetGoalActor() != MoveTarget)
	{
		MoveTarget->SetTargetActor(MoveRequest.GetGoalActor(), FSetMovementTargetKey());
		ModifiedRequest.ForceSetGoalActor(MoveTarget);
		ModifiedRequest.SetAcceptanceRadius(ModifiedRequest.GetAcceptanceRadius() +
			MoveRequest.GetGoalActor()->GetSimpleCollisionRadius());
	}
	else ModifiedRequest.ForceSetGoalActor(MoveRequest.GetGoalActor());
	return Super::MoveTo(ModifiedRequest, OutPath);
}

void AOpponentController::ReleaseAggressionToken(FReleaseTokenKey Key)
{
	Blackboard->SetValueAsBool("IsActiveCombat", false);
	CombatManager->ReleaseAggressionTokens(ControlledOpponent, FManageAggressionTokensKey());
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
	if(IsValid(CombatManager))
		CombatManager->UnregisterCombatParticipant(ControlledOpponent, FManageCombatParticipantsKey());
}

void AOpponentController::OnPossess(APawn* InPawn)
{
	RunBehaviorTree(DefaultBehaviorTree);
	Super::OnPossess(InPawn);

	//Set blackboard default values (they can't have a default value in the editor)
	
	Blackboard->SetValueAsBool("IsActiveCombat", false);
	Blackboard->SetValueAsBool("HasSensedPlayer", false);
	
	ControlledOpponent = CastChecked<AOpponentCharacter>(InPawn);
	ControlledOpponent->SetLocalFieldOfView(GetFieldOfView(), FSetFieldOfViewKey());
	ControlledOpponent->OnAggressionTokensGranted.AddDynamic(this, &AOpponentController::OnAggressionTokenGranted);

	//Setup move target
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
	if(!IsValid(Player)) return;
	if(!CombatManager->RegisterCombatParticipant(ControlledOpponent, FManageCombatParticipantsKey())) return;
	Blackboard->SetValueAsBool("HasSensedPlayer", true);
	Blackboard->SetValueAsObject("TargetObject", Player);
	ControlledOpponent->RegisterPlayerOpponent(Player, FSetPlayerOpponentKey());
}

void AOpponentController::UnregisterSensedPlayer(AActor* Player)
{
	if(!IsValid(Player)) return;
	Blackboard->SetValueAsBool("IsActiveCombat", false);
	Blackboard->SetValueAsBool("HasSensedPlayer", false);
	Blackboard->ClearValue("TargetObject");
	CombatManager->UnregisterCombatParticipant(ControlledOpponent, FManageCombatParticipantsKey());
}


void AOpponentController::OnTargetPerceptionUpdated(AActor* UpdatedActor, FAIStimulus Stimulus)
{
	//TODO: expand functionality to also detect other opponents
	const APlayerCharacter* DetectedPlayer = Cast<APlayerCharacter>(UpdatedActor);
	if(!IsValid(DetectedPlayer)) return;
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

void AOpponentController::OnAggressionTokenGranted()
{
	Blackboard->SetValueAsBool("IsActiveCombat", true);
}

#if WITH_EDITORONLY_DATA
void AOpponentController::ToggleDebugging() const
{
	MoveTarget->SetIsDebugging(!MoveTarget->GetIsDebugging());
}
#endif
