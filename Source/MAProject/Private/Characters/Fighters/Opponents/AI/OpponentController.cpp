// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/OpponentController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Utility/CombatManager.h"
#include "Characters/Fighters/Opponents/OpponentCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Prediction.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Touch.h"
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

FTimestampedStimulus::FTimestampedStimulus(const FAIStimulus& NewStimulus, double CurrentTime, AActor* Target) :
	Timestamp(CurrentTime), TargetActor(Target)
{
	SetStimulusAge(NewStimulus.GetAge());
	//TODO: find a way to copy expiration age
	Strength = NewStimulus.Strength;
	StimulusLocation = NewStimulus.StimulusLocation;
	ReceiverLocation = NewStimulus.ReceiverLocation;
	Tag = NewStimulus.Tag;
	Type = NewStimulus.Type;
	bWantsToNotifyOnlyOnValueChange = NewStimulus.WantsToNotifyOnlyOnPerceptionChange();
	bSuccessfullySensed = NewStimulus.WasSuccessfullySensed();
	bExpired = NewStimulus.IsExpired();
}

bool FTimestampedStimulus::operator==(const FTimestampedStimulus& Comp) const
{
	return Timestamp == Comp.Timestamp && TargetActor == Comp.TargetActor && GetAge() == Comp.GetAge() &&
		Strength == Comp.Strength && StimulusLocation == Comp.StimulusLocation &&
			ReceiverLocation == Comp.ReceiverLocation && Tag == Comp.Tag && Type == Comp.Type &&
				bWantsToNotifyOnlyOnValueChange == Comp.WantsToNotifyOnlyOnPerceptionChange() &&
					bSuccessfullySensed == Comp.WasSuccessfullySensed() && bExpired == Comp.IsExpired();;
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
		if(IsValid(CombatManager))
			CombatManager->UnregisterCombatParticipant(ControlledOpponent, FManageCombatParticipantsKey());
		if(IsValid(MoveTarget)) MoveTarget->Destroy();
	}
}

void AOpponentController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//Senses don't get updated except when registered/unregistered so we have to notify on position changes ourselves
	//Currently Sight is the only sense whose location can be changed
	for(FTimestampedStimulus LastSightStimulus : LastSightStimuli)
	{
		AActor* PerceivedActor = LastSightStimulus.TargetActor;
		//We shouldn't update if we have lost sight but the stimulus has jet to fade
		if(!LastSightStimulus.WasSuccessfullySensed())
		{
			//the automatic expiration system is buggy so this is a custom implementation of just that
			if(GetWorld()->TimeSeconds - LastSightStimulus.Timestamp > 1.f) OnSightForgotten(PerceivedActor);		
			continue;
		}
		
		//don't update every frame but just when relevant changes occur
		if(FVector::Distance(LastSightStimulus.StimulusLocation, PerceivedActor->GetActorLocation()) >= 100.f)
		{
			LastSightStimulus.ReceiverLocation = GetPawn()->GetActorLocation();
			LastSightStimulus.StimulusLocation = PerceivedActor->GetActorLocation();
			OnTargetPerceptionUpdated(PerceivedActor, static_cast<FAIStimulus>(LastSightStimulus));
		}
	}
}

bool AOpponentController::GenerateCombatLocation(FVector& OptimalLocation, ECombatParticipantStatus ParticipantStatus) const
{
	checkf(ParticipantStatus != ECombatParticipantStatus::NotRegistered &&
		ParticipantStatus != ECombatParticipantStatus::Player, TEXT("Getting optimal location in non-combat is not yet supported"));

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
	

	//Set blackboard default values, just to be certain (they can't have a default value in the editor)
	Blackboard->SetValueAsBool(IsActiveCombatKeyName, false);
	Blackboard->SetValueAsBool(IsInvestigatingKeyName, false);
	
	ControlledOpponent = CastChecked<AOpponentCharacter>(InPawn);
	InternalTeamId = ControlledOpponent->GetGenericTeamId();
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

void AOpponentController::TriggerInvestigationProcess(const FAIStimulus& KnownInformation)
{
	//Investigations are less important than Combat actions and cannot override existing investigations
	if(CombatManager->GetParticipationStatus(ControlledOpponent) != ECombatParticipantStatus::NotRegistered ||
		Blackboard->GetValueAsBool(IsInvestigatingKeyName)) return;

	Blackboard->SetValueAsBool(IsInvestigatingKeyName, true);
	Blackboard->SetValueAsVector(TargetLocationKeyName, KnownInformation.StimulusLocation);
}

void AOpponentController::ActiveUpdateCombat(AActor* CombatTarget, const FAIStimulus& KnownInformation)
{
	if(!IsValid(CombatTarget))
	{
		checkNoEntry();
		return;
	}

	//Enter combat if not already in it
	if(CombatManager->GetParticipationStatus(ControlledOpponent) == ECombatParticipantStatus::NotRegistered)
	{
		if(!CombatManager->RegisterCombatParticipant(ControlledOpponent, FManageCombatParticipantsKey())) return;
		ControlledOpponent->RegisterPlayerOpponent(CombatTarget->GetInstigatorController(), FSetPlayerOpponentKey());

		Blackboard->SetValueAsBool(IsInCombatKeyName, true);
		Blackboard->SetValueAsBool(IsInvestigatingKeyName, false); //cannot do both at the same time	
	}

	//Update the target location
	FVector TargetLocation;
	GenerateCombatLocation(TargetLocation, CombatManager->GetParticipationStatus(ControlledOpponent));
	Blackboard->SetValueAsVector(TargetLocationKeyName, TargetLocation);
}

void AOpponentController::EndCombat()
{
	check(CombatManager->GetParticipationStatus(ControlledOpponent) != ECombatParticipantStatus::NotRegistered);
	Blackboard->SetValueAsBool(IsInCombatKeyName, false);
	Blackboard->SetValueAsBool(IsActiveCombatKeyName, false);
	Blackboard->SetValueAsBool(RestartPatrolPathKeyName, true);

	ControlledOpponent->RegisterPlayerOpponent(nullptr, FSetPlayerOpponentKey());
	CombatManager->UnregisterCombatParticipant(ControlledOpponent, FManageCombatParticipantsKey());
}

void AOpponentController::OnSightForgotten(AActor* SightedActor)
{
	//End combat if we are in combat against the target
	if(ControlledOpponent->GetRegisteredPlayerOpponent() != nullptr &&
		ControlledOpponent->GetRegisteredPlayerOpponent() == SightedActor->GetInstigatorController())
	{
		EndCombat();
		UAISense_Prediction::RequestPawnPredictionEvent(ControlledOpponent, SightedActor, 1.f);
	}
	LastSightStimuli.RemoveAll([SightedActor](const FTimestampedStimulus& TimestampedStimulus)
	{
		return TimestampedStimulus.TargetActor == SightedActor;
	});
}


void AOpponentController::OnTargetPerceptionUpdated(AActor* UpdatedActor, FAIStimulus Stimulus)
{	
	
	const ETeamAttitude::Type AttitudeTowardsUpdatedActor =
		FGenericTeamId::GetAttitude(this, UpdatedActor->GetInstigatorController());

	if(Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		FTimestampedStimulus* MatchingStimulus = LastSightStimuli.FindByPredicate(
				[UpdatedActor](const FTimestampedStimulus& TimestampedStimulus)
				{
					return TimestampedStimulus.TargetActor == UpdatedActor;
				});
		if(MatchingStimulus == nullptr) LastSightStimuli.Add(FTimestampedStimulus(Stimulus, GetWorld()->TimeSeconds, UpdatedActor));
		else *MatchingStimulus = FTimestampedStimulus(Stimulus, GetWorld()->TimeSeconds, UpdatedActor);
		
		//Start combat
		if(AttitudeTowardsUpdatedActor != ETeamAttitude::Hostile) return;
		ActiveUpdateCombat(UpdatedActor, Stimulus);
	}
	else if(Stimulus.Type == UAISense::GetSenseID<UAISense_Prediction>())
	{
		TriggerInvestigationProcess(Stimulus);
	}
	else if(Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		//TODO: add intensity dependant logic
		//RegisterSensedPlayer(UpdatedActor->GetInstigatorController());
		//TODO: add tag dependant logic
		if(AttitudeTowardsUpdatedActor != ETeamAttitude::Hostile) return;
		TriggerInvestigationProcess(Stimulus);
	
	}
	else if(Stimulus.Type == UAISense::GetSenseID<UAISense_Touch>())
	{
		if(AttitudeTowardsUpdatedActor != ETeamAttitude::Hostile) return;
		TriggerInvestigationProcess(Stimulus);
	}
	else if(Stimulus.Type == UAISense::GetSenseID<UAISense_Damage>())
	{
		//entities that damage are always hostile
		TriggerInvestigationProcess(Stimulus);
	}
	else unimplemented();
}

void AOpponentController::OnAggressionTokenGranted()
{
	Blackboard->SetValueAsBool(IsActiveCombatKeyName, true);
}

void AOpponentController::OnAggressionTokenReleased()
{
	GLog->Log("Released");
	Blackboard->SetValueAsBool(IsActiveCombatKeyName, false);
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
