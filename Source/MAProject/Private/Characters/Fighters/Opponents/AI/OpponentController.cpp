// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/AI/OpponentController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Utility/CombatManager.h"
#include "Characters/Fighters/Opponents/OpponentCharacter.h"
#include "Components/ShapeComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
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

AOpponentController::AOpponentController() : ForwardSampleNumber(25.f), DefaultBehaviorTree(nullptr)
{
	PrimaryActorTick.bCanEverTick = true; //necessary for pawn orientation
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));

	//Register OnPerceptionUpdated delegate
	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AOpponentController::OnTargetPerceptionUpdated);
	
}

bool AOpponentController::GetOptimalLocation(FVector& OptimalLocation) const
{
	//If the controlled character is participating in combat
	if(const ECombatParticipantStatus ParticipantStatus = CombatManager->IsParticipant(ControlledOpponent);
		ParticipantStatus != ECombatParticipantStatus::NotRegistered)
	{
		//Add the constraints that are specific to this entity
		FPlayerDistanceConstraint PlayerDistanceConstraint;
		switch(ParticipantStatus)
		{
		case ECombatParticipantStatus::Active:
			{
				PlayerDistanceConstraint = *ControlledOpponent->GetActivePlayerDistanceConstraint();
				break;
			}
		case ECombatParticipantStatus::Passive:
			{
				
				PlayerDistanceConstraint = *ControlledOpponent->GetPassivePlayerDistanceConstraint();
				break;
			}
		default: checkNoEntry();
		}


		//Firstly we check whether the current position has an acceptable distance to the target
		bool IsCurrentPositionValid = true;
		const FVector CurrentLocation = ControlledOpponent->GetActorLocation();
		UShapeComponent* RequiredSpace = ControlledOpponent->GetRequiredSpace();
		if(PlayerDistanceConstraint.IsConstraintSatisfied(CurrentLocation)){
			TArray<UPrimitiveComponent*> OverlappingComponents;
			RequiredSpace->GetOverlappingComponents(OverlappingComponents);
			for(const UPrimitiveComponent* Component : OverlappingComponents)
			{
				if(Component->GetOwner() == CombatManager->GetPlayerCharacter() || Component->GetOwner()
					== ControlledOpponent) continue;
				IsCurrentPositionValid = false;
				break;
			}
		}
		else IsCurrentPositionValid = false;
		
		if(IsCurrentPositionValid)
		{
			OptimalLocation = CurrentLocation;
			return true;
		}

		FPlayerRelativeWorldZoneConstraint PlayerZoneConstraint(CombatManager->GetPlayerCharacter()->GetController(),
			CurrentLocation);

		FVector Location;
		FRotator Rotation;
		ControlledOpponent->GetActorEyesViewPoint(Location, Rotation);
		FVector ProjectedRotation = Rotation.Vector();
		ProjectedRotation.Z = 0;
		ProjectedRotation.Normalize();
		
		//TODO: Maybe try again without the zone constraint if that fails
		const FVector OpponentToTarget = CombatManager->GetPlayerCharacter()->GetActorLocation() - CurrentLocation;
		const float SampleRange = 500.f + OpponentToTarget.Length();
		const bool FoundLocation = CustomHelperFunctions::SampleGetClosestValid(OptimalLocation, RequiredSpace,
			ControlledOpponent,{CombatManager->GetPlayerCharacter()}, Location,
			ProjectedRotation * SampleRange / ForwardSampleNumber, 50.f,
			{&PlayerDistanceConstraint, &PlayerZoneConstraint},
			SampleRange, abs(OpponentToTarget.Z) + 100.f, GetWorld()
#if WITH_EDITORONLY_DATA
			, bIsDebugging
#endif
		);
		return FoundLocation;
	}
	else
	{
		UE_LOG(LogController, Warning, TEXT("Getting optimal location in non-combat is not yet supported"));
		return false;
	}
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
	ControlledOpponent->GetOnAggressionTokensGranted(FEditOnAggressionTokensGrantedOrReleasedKey()).
		AddDynamic(this, &AOpponentController::OnAggressionTokenGranted);
	ControlledOpponent->GetOnAggressionTokensReleased(FEditOnAggressionTokensGrantedOrReleasedKey()).
		AddDynamic(this, &AOpponentController::OnAggressionTokenReleased);

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

void AOpponentController::RegisterSensedPlayer(AController* Player)
{
	if(!IsValid(Player)) return;
	if(!CombatManager->RegisterCombatParticipant(ControlledOpponent, FManageCombatParticipantsKey())) return;
	Blackboard->SetValueAsBool("HasSensedPlayer", true);
	Blackboard->SetValueAsObject("TargetObject", Player);
	ControlledOpponent->RegisterPlayerOpponent(Player, FSetPlayerOpponentKey());
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
	//TODO: expand functionality to also detect other opponents
	const APlayerCharacter* DetectedPlayer = Cast<APlayerCharacter>(UpdatedActor);
	if(!IsValid(DetectedPlayer)) return;
	AController* Controller = DetectedPlayer->GetController();
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

void AOpponentController::OnAggressionTokenGranted()
{
	Blackboard->SetValueAsBool("IsActiveCombat", true);
}

void AOpponentController::OnAggressionTokenReleased()
{
	Blackboard->SetValueAsBool("IsActiveCombat", false);
	CombatManager->ReleaseAggressionTokens(ControlledOpponent, FManageAggressionTokensKey());
}

#if WITH_EDITORONLY_DATA
void AOpponentController::ToggleDebugging()
{
	bIsDebugging = !bIsDebugging;
	MoveTarget->SetIsDebugging(bIsDebugging);
}
#endif
