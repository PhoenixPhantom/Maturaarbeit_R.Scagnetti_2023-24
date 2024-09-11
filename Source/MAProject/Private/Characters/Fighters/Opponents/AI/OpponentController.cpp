// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/OpponentController.h"

#include "NavigationSystem.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Utility/CombatManager.h"
#include "Characters/Fighters/Opponents/OpponentCharacter.h"
#include "Components/SphereComponent.h"
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
#include "Utility/Animation/SuckToTargetComponent.h"
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
	RelevantSightPerceptionChangeRadius(100.f), ForwardSampleNumber(25.f), DefaultBehaviorTree(nullptr)
{
	PrimaryActorTick.bCanEverTick = true; //necessary for pawn orientation
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));

	CrowdFollowingComponent =
		Cast<UCrowdFollowingComponent>(GetComponentByClass(UCrowdFollowingComponent::StaticClass()));
	CrowdFollowingComponent->SetCrowdAvoidanceQuality(ECrowdAvoidanceQuality::Good);
	CrowdFollowingComponent->SetCrowdObstacleAvoidance(true);	

	//Register OnPerceptionUpdated delegate
	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AOpponentController::OnTargetPerceptionUpdated);
}

void AOpponentController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if(EndPlayReason == EEndPlayReason::Destroyed)
	{
		if(IsValid(CombatManager))
			CombatManager->UnregisterCombatParticipant(ControlledOpponent, false, FManageCombatParticipantsKey());
		if(IsValid(MoveTarget)) MoveTarget->Destroy();
	}
}

void AOpponentController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if(!LastSightStimuli.IsEmpty())
	{
		TArray<FTimestampedStimulus> PendingForgottenStimuli;
	
		//Senses don't get updated except when registered/unregistered so we have to notify on position changes ourselves
		//Currently Sight is the only sense whose location can be changed
		for(const FTimestampedStimulus& LastSightStimulus : LastSightStimuli)
		{
			AActor* PerceivedActor = LastSightStimulus.TargetActor;
			if(!IsValid(PerceivedActor)) PendingForgottenStimuli.Add(LastSightStimulus);
			else
			{
				//We shouldn't update if we have lost sight but the stimulus has jet to fade
				if(!LastSightStimulus.WasSuccessfullySensed())
				{
					//the automatic expiration system is buggy so this is a custom implementation for just that
					if(GetWorld()->TimeSeconds - LastSightStimulus.Timestamp > 1.f)
					{
						//try to forget the sight stimulus, if not possible, than we still update the stimulus (even if it has expired)
						if(OnSightForgotten(PerceivedActor))
						{
							PendingForgottenStimuli.Add(LastSightStimulus);
							continue;
						}
					}
					else continue;
				}
		
				//don't update every frame but just when relevant changes occur
				if(FVector::Distance(LastSightStimulus.StimulusLocation, PerceivedActor->GetActorLocation()) >=
					RelevantSightPerceptionChangeRadius)
				{
					FAIStimulus CurrentSightStimulus = LastSightStimulus;
					CurrentSightStimulus.ReceiverLocation = GetPawn()->GetActorLocation();
					CurrentSightStimulus.StimulusLocation = PerceivedActor->GetActorLocation();
					OnTargetPerceptionUpdated(PerceivedActor, CurrentSightStimulus);
				}
			}
		}

		//Remove all stimuli pending to be forgotten
		for(const FTimestampedStimulus& StimulusPendingRemoval : PendingForgottenStimuli)
		{
			LastSightStimuli.RemoveSwap(StimulusPendingRemoval);
		}
	}
}

bool AOpponentController::UpdateCombatLocation(FVector& ResultingLocation, ECombatParticipantStatus ParticipantStatus,
                                               bool ForceRecalculation) const
{
	//if the controlled character has no combat target anymore or is not in combat, this calculation is not needed
	if(!IsValid(ControlledOpponent->GetCombatTarget()))
	{
		EndCombat(true);
		return false;
	}
	const FVector CurrentLocation = ControlledOpponent->GetActorLocation();

	TArray<FReservedSpaceConstraint> ReservedSpaceConstraints;
	FCircularDistanceConstraint PlayerDistanceConstraint;
	switch(ParticipantStatus)
	{
	case ECombatParticipantStatus::Active:
		{
			PlayerDistanceConstraint = ControlledOpponent->GetActivePlayerDistanceConstraint();
			bool IsPossible = true;
			if(PlayerDistanceConstraint.bUseNavPath)
			{
				//if the distance constraint requires a connecting navigation path, a path from the NPC to the target has to exist
				const UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());
				const ANavigationData* NavData =
					NavigationSystem->GetNavDataForProps(ControlledOpponent->GetNavAgentPropertiesRef(),
						ControlledOpponent->GetNavAgentLocation());
				
				if (IsValid(NavData))
				{
					IsPossible = NavigationSystem->TestPathSync(
						FPathFindingQuery(this, *NavData,ControlledOpponent->GetNavAgentLocation(),
						PlayerDistanceConstraint.AnchorController->GetCharacter()->GetNavAgentLocation()));
				}
				else IsPossible = false;
			}
			if(!IsPossible) return false;
			break;
		}
	case ECombatParticipantStatus::Passive:
		{
			PlayerDistanceConstraint = ControlledOpponent->GetPassivePlayerDistanceConstraint();
			break;
		}
	default:
		{
			checkf(false, TEXT("Getting combat location outside combat is not supported"));
			return false;
		}
	}

	const float MinRequestedRadius = ControlledOpponent->GetRequiredSpace().GetMinimalRadius();
	TArray<AOpponentCharacter*> CombatParticipants = CombatManager->GetAllActiveParticipants();
	CombatParticipants.Append(CombatManager->GetAllPassiveParticipants());
	CombatParticipants.RemoveSwap(ControlledOpponent);
	for(const AOpponentCharacter* CombatParticipant : CombatParticipants)
	{
		FVector ParticipantTargetLocation = GetCharacterTargetLocation(CombatParticipant, TargetLocationKeyName);
		if(ParticipantTargetLocation.ContainsNaN()) continue;
		const ACharacter* CombatTarget = CombatParticipant->GetCombatTarget();
		if(!IsValid(CombatTarget)) continue;
				
		ReservedSpaceConstraints.Add(FReservedSpaceConstraint(CombatParticipant->GetRequiredSpace(),
			ParticipantTargetLocation,CombatTarget->GetActorLocation(),
			MinRequestedRadius));
	}


	//add all the calculated constraints to the array of all relevant constraints
	TArray<const FPositionalConstraint*> RelevantConstraints = {&PlayerDistanceConstraint};
	for(const FReservedSpaceConstraint& ReservedSpace : ReservedSpaceConstraints)
	{
		RelevantConstraints.Add(&ReservedSpace);
	}
	
	const FVector CurrentToCombatTarget = ControlledOpponent->GetCombatTarget()->GetActorLocation() - CurrentLocation;
	if(!ForceRecalculation)
	{
		//Check if it is necessary to change the target location, before calculating it, reducing movement noise
		FVector CurrentTargetLocation;
		if(Blackboard->GetValueAsBool(HasJustExecutedAttackKeyName))
		{
			//if the character has executed an attack, after reaching the target location,
			//the character is not walking anymore is most likely not in the same position as the
			//last walk to target anymore (due to the SuckToTargetComponent). The current position is then more relevant
			//and there is no target point interpolation needed as the character is not moving currently anyways
			CurrentTargetLocation = CurrentLocation;
			MoveTarget->ForceNoInterpolationOnce();
		}
		else CurrentTargetLocation = Blackboard->GetValueAsVector(TargetLocationKeyName);
		
		if(UConstraintsFunctionLibrary::GetMatchLevel(CurrentTargetLocation, RelevantConstraints, GetWorld(),
			FVector(0, 0, abs(CurrentToCombatTarget.Z) + 100.f),
			UConstraintsFunctionLibrary::RequireAllValid))
		{
			ResultingLocation = CurrentTargetLocation;
			return true;
		}
	}

#if WITH_EDITORONLY_DATA
	if(bIsDebugging){
		GLog->Log(GetActorNameOrLabel() + " is recalculating desired location");
	}
#endif


	const FPlayerRelativeWorldZoneConstraint PlayerZoneConstraint(ControlledOpponent->GetCombatTargetController(),
	                                                              CurrentLocation);
	
	RelevantConstraints.Add(&PlayerZoneConstraint);

	const FCircularPointsGenerator PointGenerator(PlayerDistanceConstraint, -CurrentToCombatTarget,
	                                             ECombatParticipantStatus::Active == ParticipantStatus ? 0.05 : 0.001);
	const bool FoundLocation = UConstraintsFunctionLibrary::GetBestPositionSampled(ResultingLocation, PointGenerator,
		RelevantConstraints, GetWorld(),FVector(0.0, 0.0, abs(CurrentToCombatTarget.Z) + 500.f),
		UConstraintsFunctionLibrary::RequireAllOptimal,
		UConstraintsFunctionLibrary::RequireAllValid, false
#if WITH_EDITORONLY_DATA
		, bIsDebugging
#endif
	);
#if WITH_EDITORONLY_DATA
	if(!FoundLocation)
	{
		GLog->Log("Repositioning failed");
	}
#endif
	
	Blackboard->SetValueAsBool(HasJustExecutedAttackKeyName, false);
	return FoundLocation;
}

FPathFollowingRequestResult AOpponentController::MoveTo(const FAIMoveRequest& MoveRequest, FNavPathSharedPtr* OutPath)
{
	//Smooth transitions are only required in combat as that is the place where quick changes to the target location
	//can be made
	if(!IsValid(MoveTarget) ||
		CombatManager->GetParticipationStatus(ControlledOpponent) != ECombatParticipantStatus::Active)
	{
		if(IsValid(MoveTarget)) MoveTarget->SetMovementTargetLocation(FAISystem::InvalidLocation, FSetMovementTargetKey());
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

	const FPathFollowingRequestResult PathFollowingRequestResult = Super::MoveTo(ModifiedRequest, OutPath);

	if(PathFollowingRequestResult.Code == EPathFollowingRequestResult::RequestSuccessful)
	{
		//Use the original request, as the new one generally doesn't contain the "actual" target position
		//(as the modified one interpolates to the new target position over time)
		ControlledOpponent->GetCharacterRotationManager()->ChooseOptimalForCombat(MoveRequest.GetGoalLocation());
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
	Blackboard->SetValueAsBool(RestartPatrolPathKeyName, true);
	
	ControlledOpponent = CastChecked<AOpponentCharacter>(InPawn);
	ControlledOpponent->SetUsedBlackboardComponent(Blackboard, FSetUsedBlackboardKey());
	InternalTeamId = ControlledOpponent->GetGenericTeamId();
	ControlledOpponent->SetLocalFieldOfView(GetFieldOfView(), FSetFieldOfViewKey());
	
	TDelegate<void()> OnTokensGranted;
	OnTokensGranted.BindUObject(this, &AOpponentController::OnAggressionTokenGranted);
	ControlledOpponent->BindOnAggressionTokensGranted(OnTokensGranted, FEditOnAggressionTokensGrantedOrReleasedKey());
	
	TDelegate<void()> OnTokensReleased;
	OnTokensReleased.BindUObject(this, &AOpponentController::OnAggressionTokenReleased);
	ControlledOpponent->BindOnAggressionTokensReleased(OnTokensReleased, FEditOnAggressionTokensGrantedOrReleasedKey());

	
	SetActorLabel(ControlledOpponent->GetActorNameOrLabel() + " Controller");

	//Setup move target
	if(!IsValid(MoveTarget))
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParameters.Name = ToCStr(GetActorNameOrLabel() + "_MovementTarget");
		MoveTarget = GetWorld()->SpawnActor<AMovementTarget>(MovementTargetClass, SpawnParameters);
	}
	
	MoveTarget->SetActorLabel(ControlledOpponent->GetActorNameOrLabel() + " MovementTarget");
	MoveTarget->SetActorLocation(GetPawn()->GetActorLocation());
}

void AOpponentController::TriggerInvestigationProcess(const FAIStimulus& KnownInformation) const
{
	//Investigations are less important than Combat actions and cannot override existing investigations
	if(CombatManager->GetParticipationStatus(ControlledOpponent) != ECombatParticipantStatus::NotRegistered ||
		Blackboard->GetValueAsBool(IsInvestigatingKeyName)) return;

	Blackboard->SetValueAsBool(IsInvestigatingKeyName, true);
	Blackboard->SetValueAsVector(TargetLocationKeyName, KnownInformation.StimulusLocation);
}

void AOpponentController::ActiveUpdateCombat(const AActor* CombatTarget, const FAIStimulus& KnownInformation) const
{
	if(!IsValid(CombatTarget))
	{
		checkNoEntry();
		return;
	}

	
	//Enter combat if not already in it, or if previously fighting someone else
	if(CombatManager->GetParticipationStatus(ControlledOpponent) == ECombatParticipantStatus::NotRegistered ||
		CombatTarget->GetInstigatorController() != ControlledOpponent->GetCombatTargetController())
	{
		ControlledOpponent->RegisterCombatTarget(CombatTarget->GetInstigatorController(), FSetCombatTargetKey());
		CombatManager->RegisterCombatParticipant(ControlledOpponent, FManageCombatParticipantsKey());

		Blackboard->SetValueAsBool(IsInCombatKeyName, true);
		Blackboard->SetValueAsBool(IsInvestigatingKeyName, false); //cannot do both at the same time
	}

	//Update the target location
	FVector TargetLocation;
	UpdateCombatLocation(TargetLocation, CombatManager->GetParticipationStatus(ControlledOpponent), false);
	Blackboard->SetValueAsVector(TargetLocationKeyName, TargetLocation);
}

void AOpponentController::EndCombat(bool FullyUnregister) const
{
	check(CombatManager->GetParticipationStatus(ControlledOpponent) != ECombatParticipantStatus::NotRegistered);
	Blackboard->SetValueAsBool(IsInCombatKeyName, false);
	Blackboard->SetValueAsEnum(LastCombatStatusKeyName, static_cast<uint8>(ECombatParticipantStatus::NotRegistered));
	Blackboard->SetValueAsBool(IsActiveCombatKeyName, false);
	Blackboard->SetValueAsBool(HasJustExecutedAttackKeyName, false);
	Blackboard->SetValueAsBool(RestartPatrolPathKeyName, true);

	ControlledOpponent->RegisterCombatTarget(nullptr, FSetCombatTargetKey());
	CombatManager->UnregisterCombatParticipant(ControlledOpponent, !FullyUnregister, FManageCombatParticipantsKey());
#if WITH_EDITORONLY_DATA
	if(bIsDebugging) GLog->Log(ControlledOpponent->GetActorNameOrLabel() + " has ended combat.");
#endif
}

bool AOpponentController::OnSightForgotten(AActor* SightedActor) const
{
	//End combat if we are in combat against the target or if the target already expired
	if(ControlledOpponent->GetCombatTargetController() != nullptr &&
		ControlledOpponent->GetCombatTargetController() == SightedActor->GetInstigatorController())
	{
		//if the target is so close, that we can touch it, it is nonsensical to loose sight of it
		//rather, the character should turn to face the target
		if(FVector::Distance(SightedActor->GetActorLocation(), ControlledOpponent->GetActorLocation()) <=
			ControlledOpponent->GetSimpleCollisionRadius() + SightedActor->GetSimpleCollisionRadius() + 500.f)
		{
			ControlledOpponent->GetCharacterRotationManager()->SetRotationMode(ECharacterRotationMode::OrientToTarget,
			false, nullptr, SightedActor->GetActorLocation());
			return false;
		}
		//else
		EndCombat();
		UAISense_Prediction::RequestPawnPredictionEvent(ControlledOpponent, SightedActor, 1.f);
	}
	return true;
}

FVector AOpponentController::GetCharacterTargetLocation(const AOpponentCharacter* RelevantCharacter,
	FName BlackboardTargetLocationName)
{
	FVector ParticipantTargetLocation;
	//determine the target location if moving, otherwise the current location
	if(RelevantCharacter->GetCurrentMontage() != nullptr)
	{
		ParticipantTargetLocation = RelevantCharacter->GetSuckToTargetComponent()->GetTargetLocation();
	}
	else
	{
		//See if the character is moving.
		//As the squared length is slightly faster, we use that. We actually mean velocity > 10cm/s
		if(RelevantCharacter->GetVelocity().SquaredLength() > 100.0)
		{
			ParticipantTargetLocation =
				RelevantCharacter->GetUsedBlackboardComponent()->GetValueAsVector(BlackboardTargetLocationName);
		}
		else
		{
			ParticipantTargetLocation = RelevantCharacter->GetActorLocation();
		}
	}
	return ParticipantTargetLocation;
}

// ReSharper disable once CppPassValueParameterByConstReference
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
		if(MatchingStimulus == nullptr)
		{
			LastSightStimuli.Add(FTimestampedStimulus(Stimulus, GetWorld()->TimeSeconds, UpdatedActor));
			TooCloseToForgetStimuli.RemoveAllSwap([UpdatedActor](const FTimestampedStimulus& TimestampedStimulus)
				{
					return TimestampedStimulus.TargetActor == UpdatedActor;
				});
		}
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

// ReSharper disable once CppMemberFunctionMayBeConst
void AOpponentController::OnAggressionTokenGranted()
{
#if WITH_EDITORONLY_DATA
	if(bIsDebugging) GLog->Log(GetActorNameOrLabel() + " received an aggression token");
#endif
	Blackboard->SetValueAsBool(IsActiveCombatKeyName, true);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AOpponentController::OnAggressionTokenReleased()
{
#if WITH_EDITORONLY_DATA
	if(bIsDebugging) GLog->Log(GetActorNameOrLabel() + " released it's aggression token");
#endif
	Blackboard->SetValueAsBool(IsActiveCombatKeyName, false);
	CombatManager->ReleaseAggressionTokens(ControlledOpponent, FManageAggressionTokensKey());
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AOpponentController::OnFlickBackTriggered(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	ControlledOpponent->GetCharacterRotationManager()->SetRotationMode(ECharacterRotationMode::FlickBack);
}

#if WITH_EDITORONLY_DATA
void AOpponentController::ToggleDebugging()
{
	bIsDebugging = !bIsDebugging;
	MoveTarget->SetIsDebugging(bIsDebugging);
	ControlledOpponent->SetIsDebugging(bIsDebugging);
}
#endif
