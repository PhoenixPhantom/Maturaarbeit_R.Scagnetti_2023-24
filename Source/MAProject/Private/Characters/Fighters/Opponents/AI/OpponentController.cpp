// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/OpponentController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Utility/CombatManager.h"
#include "Characters/Fighters/Opponents/OpponentCharacter.h"
#include "Components/ShapeComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Utility/NonPlayerFunctionality/CharacterRotationManagerComponent.h"
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

AOpponentController::AOpponentController(const FObjectInitializer& ObjectInitializer) :
Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent"))),
ForwardSampleNumber(25.f), DefaultBehaviorTree(nullptr)
{
	PrimaryActorTick.bCanEverTick = true; //necessary for pawn orientation
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));

	CrowdFollowingComponent =
		Cast<UCrowdFollowingComponent>(GetComponentByClass(UCrowdFollowingComponent::StaticClass()));

	//Register OnPerceptionUpdated delegate
	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AOpponentController::OnTargetPerceptionUpdated);
}

void AOpponentController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if(EndPlayReason == EEndPlayReason::Destroyed)
	{
		GetWorld()->GetTimerManager().ClearTimer(LostPerceptionHandle);
		if(IsValid(CombatManager))
			CombatManager->UnregisterCombatParticipant(ControlledOpponent, FManageCombatParticipantsKey());
		if(IsValid(MoveTarget)) MoveTarget->Destroy();
	}
}

bool AOpponentController::GenerateTargetLocation(FVector& OptimalLocation) const
{
	//If the controlled character is participating in combat
	if(const ECombatParticipantStatus ParticipantStatus = CombatManager->GetParticipationStatus(ControlledOpponent);
		ParticipantStatus != ECombatParticipantStatus::NotRegistered)
	{
		const FVector CurrentLocation = ControlledOpponent->GetActorLocation();
		bool RequireNavData = false;
		

		
		FCircularDistanceConstraint PlayerDistanceConstraint;
		switch(ParticipantStatus)
		{
		case ECombatParticipantStatus::Active:
			{
				PlayerDistanceConstraint = ControlledOpponent->GetActivePlayerDistanceConstraint();
				RequireNavData = false;
				break;
			}
		case ECombatParticipantStatus::Passive:
			{
				
				PlayerDistanceConstraint = ControlledOpponent->GetPassivePlayerDistanceConstraint();
				break;
			}
		default: checkNoEntry();
		}

		FPlayerRelativeWorldZoneConstraint PlayerZoneConstraint(CombatManager->GetPlayerCharacter()->GetController(),
			CurrentLocation);
		
		FObstacleSpaceConstraint ObstacleSpaceConstraint(ControlledOpponent->GetRequiredSpace(),
			{CombatManager->GetPlayerCharacter(), ControlledOpponent},
			PlayerDistanceConstraint.GetMaxMatchLevel() + PlayerZoneConstraint.GetMaxMatchLevel() - 1);


		
		FVector Location;
		FRotator Rotation;
		ControlledOpponent->GetActorEyesViewPoint(Location, Rotation);
		FVector ProjectedRotation = Rotation.Vector();
		ProjectedRotation.Z = 0;
		ProjectedRotation.Normalize();
		
		const FVector OpponentToTarget = CombatManager->GetPlayerCharacter()->GetActorLocation() - CurrentLocation;
		const float SampleRange = 500.f + OpponentToTarget.Length();
		
		const bool FoundLocation = CustomHelperFunctions::SampleGetClosestValid(OptimalLocation, CurrentLocation,
			ProjectedRotation * SampleRange / ForwardSampleNumber, 100.f,
			{&PlayerDistanceConstraint, &PlayerZoneConstraint, &ObstacleSpaceConstraint},
			SampleRange, abs(OpponentToTarget.Z) + 100.f, GetWorld(), RequireNavData
#if WITH_EDITORONLY_DATA
			, bIsDebugging
#endif
		);
		
		return FoundLocation;
	}

	
	UE_LOG(LogController, Warning, TEXT("Getting optimal location in non-combat is not yet supported"));
	return false;
}

bool AOpponentController::IsValidTargetLocation(const FVector& TargetLocation) const
{
	const ECombatParticipantStatus ParticipantStatus = CombatManager->GetParticipationStatus(ControlledOpponent);
	if(ParticipantStatus == ECombatParticipantStatus::NotRegistered) return false;
	
	bool RequireNavData = false;
	//Add the constraints that are specific to this entity
	FCircularDistanceConstraint PlayerDistanceConstraint;
	switch(ParticipantStatus)
	{
	case ECombatParticipantStatus::Active:
		{
			PlayerDistanceConstraint = ControlledOpponent->GetActivePlayerDistanceConstraint();
			RequireNavData = false;
			break;
		}
	case ECombatParticipantStatus::Passive:
		{
			
			PlayerDistanceConstraint = ControlledOpponent->GetPassivePlayerDistanceConstraint();
			break;
		}
	default: checkNoEntry();
	}

	//whether the current position has an acceptable relation to the target
	const UShapeComponent* RequiredSpace = ControlledOpponent->GetRequiredSpace();
	if(PlayerDistanceConstraint.IsConstraintSatisfied(TargetLocation, RequireNavData)){
		TArray<UPrimitiveComponent*> OverlappingComponents;
		RequiredSpace->GetOverlappingComponents(OverlappingComponents);
		for(const UPrimitiveComponent* Component : OverlappingComponents)
		{
			if(Component->GetOwner() == CombatManager->GetPlayerCharacter() ||
				Component->GetOwner() == ControlledOpponent) continue;
			return true;
		}
	}
	return false;
}

FPathFollowingRequestResult AOpponentController::MoveTo(const FAIMoveRequest& MoveRequest, FNavPathSharedPtr* OutPath)
{
	//Smooth transitions are only required in combat as that is also the place where quick changes to the target location
	//can be made
	if(!IsValid(MoveTarget) ||
		CombatManager->GetParticipationStatus(ControlledOpponent) != ECombatParticipantStatus::Active)
	{
		MoveTarget->SetMovementTargetLocation(FAISystem::InvalidLocation, FSetMovementTargetKey());
		return Super::MoveTo(MoveRequest, OutPath);
	}

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

	//Out path only gets written to if it is valid
	if(OutPath == nullptr || !OutPath->IsValid())
	{
		*OutPath = MakeShared<FNavigationPath, ESPMode::ThreadSafe>();
	}

	const FPathFollowingRequestResult PathFollowingRequestResult = Super::MoveTo(ModifiedRequest, OutPath);

	if(PathFollowingRequestResult.Code == EPathFollowingRequestResult::RequestSuccessful)
	{
		//Use the original request, as the new one generally doesn't contain the "actual" target position
		//(as it interpolates to the new target position over time)
		ControlledOpponent->GetCharacterRotationManager()->SwitchToOptimal(MoveRequest.GetGoalLocation(), OutPath);
		CrowdFollowingComponent->SetCrowdObstacleAvoidance(
			CombatManager->GetParticipationStatus(ControlledOpponent) == ECombatParticipantStatus::Active);
	}

	return  PathFollowingRequestResult;
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

	ReceiveMoveCompleted.AddDynamic(this, &AOpponentController::OnFlickBackTriggered);
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
	ControlledOpponent->GetOnAggressionTokensGranted(FEditOnAggressionTokensGrantedOrReleasedKey()).
		AddDynamic(this, &AOpponentController::OnAggressionTokenGranted);
	ControlledOpponent->GetOnAggressionTokensReleased(FEditOnAggressionTokensGrantedOrReleasedKey()).
		AddDynamic(this, &AOpponentController::OnAggressionTokenReleased);

	
	SetActorLabel(ControlledOpponent->GetActorNameOrLabel() + " Controller");

	//Setup move target
	if(!IsValid(MoveTarget))
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParameters.Name = ToCStr(GetActorNameOrLabel() + "_MovementTarget");
		MoveTarget = GetWorld()->SpawnActor<AMovementTarget>(AMovementTarget::StaticClass(), SpawnParameters);
	}
	
	MoveTarget->SetActorLabel(ControlledOpponent->GetActorNameOrLabel() + " MovementTarget");
	MoveTarget->SetActorLocation(GetPawn()->GetActorLocation());
}

void AOpponentController::RegisterSensedPlayer(AController* Player)
{
	if(!IsValid(Player)) return;
	
	AController* OldPlayer = ControlledOpponent->GetRegisteredPlayerOpponent();
	if(OldPlayer == Player) return; //cannot register the same player twice
	ControlledOpponent->RegisterPlayerOpponent(Player, FSetPlayerOpponentKey());
	
	if(!CombatManager->RegisterCombatParticipant(ControlledOpponent, FManageCombatParticipantsKey()))
	{
		ControlledOpponent->RegisterPlayerOpponent(OldPlayer, FSetPlayerOpponentKey());
		return;
	}
	
	Blackboard->SetValueAsBool("HasSensedPlayer", true);
	Blackboard->SetValueAsObject("TargetObject", Player);
}

void AOpponentController::UnregisterSensedPlayer(AController* Player)
{
	if(!IsValid(Player)) return;
	Blackboard->SetValueAsBool("IsActiveCombat", false);
	Blackboard->SetValueAsBool("HasSensedPlayer", false);
	Blackboard->ClearValue("TargetObject");
	ControlledOpponent->RegisterPlayerOpponent(nullptr, FSetPlayerOpponentKey());
	CombatManager->UnregisterCombatParticipant(ControlledOpponent, FManageCombatParticipantsKey());
}


void AOpponentController::OnTargetPerceptionUpdated(AActor* UpdatedActor, FAIStimulus Stimulus)
{
	const bool HasDetectedPlayer = UpdatedActor->GetClass()->IsChildOf(APlayerCharacter::StaticClass());
	
	if(Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		if(!HasDetectedPlayer) return;
		AController* Controller = CastChecked<ACharacter>(UpdatedActor)->GetController();
		if(!Stimulus.WasSuccessfullySensed())
		{
			if(!GetWorldTimerManager().TimerExists(LostPerceptionHandle)){  
				GetWorldTimerManager().SetTimer(LostPerceptionHandle, [this, Controller]
				{
					UnregisterSensedPlayer(Controller);
				}, 5.f, true);
			}
		}
		else
		{
			if(GetWorldTimerManager().TimerExists(LostPerceptionHandle)) GetWorldTimerManager().ClearTimer(LostPerceptionHandle);
			RegisterSensedPlayer(Controller);
		}
	}
	else if(Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{

		if(!HasDetectedPlayer) return;
		AController* Controller = CastChecked<ACharacter>(UpdatedActor)->GetController();
		// Stimulus.Tag
		if(!Stimulus.WasSuccessfullySensed())
		{
			if(!GetWorldTimerManager().TimerExists(LostPerceptionHandle)){  
				GetWorldTimerManager().SetTimer(LostPerceptionHandle, [this, Controller]
				{
					UnregisterSensedPlayer(Controller);
				}, 5.f, true);
			}
		}
		else
		{
			if(GetWorldTimerManager().TimerExists(LostPerceptionHandle)) GetWorldTimerManager().ClearTimer(LostPerceptionHandle);
			RegisterSensedPlayer(Controller);
		}			
	}
	else if(Stimulus.Type == UAISense::GetSenseID<UAISense_Damage>())
	{
		if(!HasDetectedPlayer) return;
		AController* Controller = CastChecked<ACharacter>(UpdatedActor)->GetController();
		if(!Stimulus.WasSuccessfullySensed())
		{
			if(!GetWorldTimerManager().TimerExists(LostPerceptionHandle)){  
				GetWorldTimerManager().SetTimer(LostPerceptionHandle, [this, Controller]
				{
					UnregisterSensedPlayer(Controller);
				}, 5.f, true);
			}
		}
		else
		{
			if(GetWorldTimerManager().TimerExists(LostPerceptionHandle)) GetWorldTimerManager().ClearTimer(LostPerceptionHandle);
			RegisterSensedPlayer(Controller);
		}
	}
	else{
		unimplemented();
	}
}

void AOpponentController::OnAggressionTokenGranted()
{
	Blackboard->SetValueAsBool("IsActiveCombat", true);
}

void AOpponentController::OnAggressionTokenReleased()
{
	Blackboard->SetValueAsBool("IsActiveCombat", false);
	CombatManager->ReleaseAggressionTokens(ControlledOpponent, FManageAggressionTokensKey());
}

void AOpponentController::OnFlickBackTriggered(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	ControlledOpponent->GetCharacterRotationManager()->SetRotationMode(ECharacterRotationMode::FlickBack);
}

#if WITH_EDITORONLY_DATA
void AOpponentController::ToggleDebugging()
{
	bIsDebugging = !bIsDebugging;
	MoveTarget->SetIsDebugging(bIsDebugging);
}
#endif
