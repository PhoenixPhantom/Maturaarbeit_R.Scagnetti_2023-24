// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/CombatManager.h"

#include "Characters/Fighters/Opponents/OpponentCharacter.h"
#include "Characters/Fighters/Player/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"


bool FAggressionData::operator==(const FAggressionData& AggressionData) const
{
	return AggressionScore == AggressionData.AggressionScore && Holder == AggressionData.Holder &&
		RequestedTokens == AggressionData.RequestedTokens;
}

// Sets default values
ACombatManager::ACombatManager() : MaxAggressionTokens(1), PreferBestScorePower(4.f)
{
#if WITH_EDITORONLY_DATA
	PrimaryActorTick.bCanEverTick = true;
#else
	PrimaryActorTick.bCanEverTick = false;
#endif
}

bool ACombatManager::GetAggressivenessDependantLocation(FVector& ResultingLocation, AOpponentCharacter* OwningCharacter)
{
	if(ECombatParticipantStatus::NotRegistered == IsParticipant(OwningCharacter))
	{
		UE_LOG(LogTemp, Warning, TEXT("Try getting agressiveness dependant location for a non-combat engaged entity."));
		return false;
	}
	const FVector OpponentLocation = OwningCharacter->GetActorLocation();


	/*TArray<const FPositionalConstraint*> RelevantConstraints = PositionalConstraints;
	//we have to remove all constraints imposed by the entity we want to move
	//(otherwise, wherever it stands will not be valid as something (itself) is already there)
	RelevantConstraints.RemoveAll([OwningCharacter](const FPositionalConstraint* Constraint)
	{ return Constraint->Owner == OwningCharacter; });*/


	TArray<const FPositionalConstraint*> RelevantConstraints;
	//Add the constraints that are specific to this entity
	FPlayerDistanceConstraint PlayerDistanceConstraint;
	if(ActiveParticipants.Contains(OwningCharacter)) PlayerDistanceConstraint = *OwningCharacter->GetActivePlayerDistanceConstraint();
	else PlayerDistanceConstraint = *OwningCharacter->GetPassivePlayerDistanceConstraint();
	RelevantConstraints.Add(&PlayerDistanceConstraint);

	FPlayerRelativeWorldZoneConstraint* PlayerZoneConstraint =
		new FPlayerRelativeWorldZoneConstraint(PlayerCharacter->GetController());
	PlayerZoneConstraint->ConstraintZone = PlayerZoneConstraint->CalculateTargetZone(OpponentLocation);
	RelevantConstraints.Add(PlayerZoneConstraint);

#if WITH_EDITORONLY_DATA
	if(bIsDebugging)
	{
		//Debug shapes should be shown for 10s (but only when there is no more recent debug shape)
		DebugImagesToDraw.Key = 10.f;
		DebugImagesToDraw.Value.Clear();
		DebugImagesToDraw.Value.AddLambda([DistanceConstraint = PlayerDistanceConstraint,
			ZoneConstraint = *PlayerZoneConstraint, World = GetWorld(),
			DCPosition = PlayerDistanceConstraint.Player->GetPawn()->GetActorLocation(),
			ZCPosition = PlayerZoneConstraint->Player->GetPawn()->GetActorLocation()]
		{
			DistanceConstraint.DrawOldConstraintDebug(World, DCPosition, FLinearColor(0, 255, 0), 0.f);
			ZoneConstraint.DrawOldConstraintDebug(World, ZCPosition, FLinearColor(0, 255, 0), 0.f);
		});
		
	}
#endif
	
	//We only have to expensively find a new location if the current one isn't good anymore
	bool AreAllSatisfied = true;
	for(const FPositionalConstraint* Constraint : RelevantConstraints)
	{
		if(!Constraint->IsConstraintSatisfied(OpponentLocation))
		{
			AreAllSatisfied = false;
			break;
		}
	}
	if(AreAllSatisfied)
	{
		ResultingLocation = OpponentLocation;
		return true;
	}

	FVector Location;
	FRotator Rotation;
	OwningCharacter->GetActorEyesViewPoint(Location, Rotation);
	FVector ProjectedRotation = Rotation.Vector();
	ProjectedRotation.Z = 0;
	ProjectedRotation.Normalize();
	PlayerDistanceConstraint.bRequireOptimal = true;
	return SampleGetClosestValid(ResultingLocation, OwningCharacter->GetActorLocation(),
	                             ProjectedRotation * 100.f, 50.f, RelevantConstraints,
	                             5000.f, GetWorld()
#if WITH_EDITORONLY_DATA
	                             , bIsDebugging
#endif
	);
}

ECombatParticipantStatus ACombatManager::IsParticipant(AFighterCharacter* Character) const
{
	if(Character == PlayerCharacter) return ECombatParticipantStatus::Player;
	if(PassiveParticipants.Contains(Character)) return ECombatParticipantStatus::Passive;
	if(ActiveParticipants.Contains(Character)) return ECombatParticipantStatus::Active;
	return ECombatParticipantStatus::NotRegistered;
}

void ACombatManager::RegisterCombatParticipant(APlayerCharacter* PlayerParticipant, FManageCombatParticipantsKey Key)
{
	PlayerCharacter = PlayerParticipant;
}

bool ACombatManager::RegisterCombatParticipant(AOpponentCharacter* Participant, FManageCombatParticipantsKey Key)
{
	if(ECombatParticipantStatus::NotRegistered != IsParticipant(Participant)) return false;
	PositionalConstraints.Add(Participant->GetPassivePositionConstraint());
	PassiveParticipants.Add(Participant);
	AttemptDistributeRemainingTokens();
	return true;
}

void ACombatManager::UnregisterCombatParticipant(AOpponentCharacter* Participant, FManageCombatParticipantsKey Key)
{
	if(AnticipatedActive.Holder == Participant) AnticipatedActive = FAggressionData();
	//we can't use ReleaseAggressionTokens as this could lead to token redistribution and Participant not becoming passive
	RemoveAggressionTokens(Participant);
	PassiveParticipants.Remove(Participant);
	PositionalConstraints.RemoveAll([Participant](const FPositionalConstraint* Constraint)
					{ return Constraint->Owner == Participant; });
	AttemptDistributeRemainingTokens();
}

void ACombatManager::ReleaseAggressionTokens(AOpponentCharacter* Participant, FManageAggressionTokensKey Key)
{
	if(RemoveAggressionTokens(Participant))	AttemptDistributeRemainingTokens();
}

#if WITH_EDITORONLY_DATA
void ACombatManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if(bIsDebugging)
	{
		for(const FPositionalConstraint* Constraint : PositionalConstraints)
		{
			Constraint->DrawConstraintDebug(GetWorld(), FLinearColor(0, 0, 0), 0);
		}

		for(const AOpponentCharacter* OpponentCharacter : PassiveParticipants)
		{
			UKismetSystemLibrary::DrawDebugCircle(GetWorld(), OpponentCharacter->GetActorLocation(), 100.f,
		50, FLinearColor(0, 0, 255), 0, 20.f, FVector(0, 1, 0),
		FVector(1, 0, 0));
		}
		for(const AOpponentCharacter* OpponentCharacter : ActiveParticipants)
		{
			UKismetSystemLibrary::DrawDebugCircle(GetWorld(), OpponentCharacter->GetActorLocation(), 100.f,
		50, FLinearColor(255, 0, 0), 0, 20.f, FVector(0, 1, 0),
		FVector(1, 0, 0));
		}
		
		if(DebugImagesToDraw.Key > 0.f)
		{
			DebugImagesToDraw.Key -= DeltaSeconds;
			DebugImagesToDraw.Value.Broadcast();
		}
	}
}
#endif

// Called when the game starts or when spawned
void ACombatManager::BeginPlay()
{
	Super::BeginPlay();

	AvailableAggressionTokens = MaxAggressionTokens;

	//There can only be one combat manager at any given time to prevent logic problems
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACombatManager::StaticClass(), Actors);
	check(Actors.Num() == 1 && Actors[0] == this);
}

bool ACombatManager::RemoveAggressionTokens(AOpponentCharacter* Participant)
{
	if(!ActiveParticipants.Contains(Participant)) return false;
	AvailableAggressionTokens += Participant->GetRequestedTokens();
	return MakePassiveParticipant(ActiveParticipants.Find(Participant));
}

bool ACombatManager::GrantTokens(const FAggressionData& AggressionData)
{
	if(!PassiveParticipants.Contains(AggressionData.Holder) || ActiveParticipants.Contains(AggressionData.Holder))
	{
		checkNoEntry();
		return false;
	}
	if(AggressionData.RequestedTokens > AvailableAggressionTokens) return false;
	AvailableAggressionTokens -= AggressionData.RequestedTokens;
	MakeActiveParticipant(PassiveParticipants.Find(AggressionData.Holder));
	AggressionData.Holder->ExecuteOnAggressionTokensGranted(FExecuteOnAggressionTokensGrantedKey());
	return true;
}

bool ACombatManager::MakeActiveParticipant(int32 Index)
{
	if(!PassiveParticipants.IsValidIndex(Index))
	{
		checkNoEntry();
		return false;
	}
	PositionalConstraints.RemoveAll(
		[Participant = PassiveParticipants[Index]](const FPositionalConstraint* Constraint)
					{ return Constraint->Owner == Participant; });
	PositionalConstraints.Add(PassiveParticipants[Index]->GetActivePositionConstraint());
	ActiveParticipants.Add(PassiveParticipants[Index]);
	PassiveParticipants.RemoveAt(Index);
	return true;
}

bool ACombatManager::MakePassiveParticipant(int32 Index)
{
	if(!ActiveParticipants.IsValidIndex(Index))
	{
		checkNoEntry();
		return false;
	}
	PositionalConstraints.RemoveAll(
		[Participant = ActiveParticipants[Index]](const FPositionalConstraint* Constraint)
					{ return Constraint->Owner == Participant; });
	PositionalConstraints.Add(ActiveParticipants[Index]->GetPassivePositionConstraint());
	PassiveParticipants.Add(ActiveParticipants[Index]);
	ActiveParticipants.RemoveAt(Index);
	return true;
}

void ACombatManager::AttemptDistributeRemainingTokens()
{
	if(IsValid(AnticipatedActive.Holder))
	{
		if(!GrantTokens(AnticipatedActive)) return;
	}
	
	//Generate the aggression score for every passive participant
	float LowestScore = std::numeric_limits<float>::max();
	TArray<FAggressionData> RelevantData;
	for(AOpponentCharacter* Participant : PassiveParticipants)
	{
		const uint32 RequestedTokens = Participant->GetRequestedTokens();
		//There is no use trying to distribute tokens to either an entity that needs more than what can be provided
		//or to an entity that doesn't need tokens
		if(RequestedTokens > MaxAggressionTokens || RequestedTokens == 0) continue;
		const float Score = Participant->GenerateAggressionScore(PlayerCharacter);
		//A score < 0.f means that the given participant cannot become aggressive, so it is not relevant
		if(Score >= 0.f)
		{
			if(LowestScore > Score) LowestScore = Score;
			//TotalAggressionScoreSquared += pow(Score, PreferBestScorePower);
			RelevantData.Add(FAggressionData(Score, Participant, RequestedTokens));
		}
	}
	double TotalScoreAdjusted = 0.0;
	for(FAggressionData& Data : RelevantData)
	{
		const double ScoreAdjusted = pow(Data.AggressionScore - LowestScore, PreferBestScorePower);
		TotalScoreAdjusted += ScoreAdjusted;
		//Save the adjusted score so we have the calculation in only one place (so changing it is more cumbersome)
		Data.AggressionScore = ScoreAdjusted;
	}
	
	while(AvailableAggressionTokens > 0 && !RelevantData.IsEmpty())
	{
		FAggressionData ChosenOption;
		//we use double in the assignment because we could be dealing with very small numbers and we still want high accuracy
		float RandomNumber = static_cast<double>(rand()) * TotalScoreAdjusted / static_cast<double>(RAND_MAX);
		for(const FAggressionData& AggressionData : RelevantData)
		{
			RandomNumber -= AggressionData.AggressionScore;
			if(RandomNumber <= 0.f)
			{
				ChosenOption = AggressionData;
				break;
			}
		}

		//Try to grant tokens to the chosen entity
		if(!GrantTokens(ChosenOption))
		{
			//if the entity cannot be granted enough tokens and other entities are can't get a token because of
			//the chosen option the entity should be guaranteed to be the next one to be granted tokens (it is anticipated)
			if(RelevantData.Num() > 1) AnticipatedActive = ChosenOption;
			break;
		}
		//if we were able to grant the token, the entity cannot be granted a token again for some time
		RelevantData.Remove(ChosenOption);
		TotalScoreAdjusted -= ChosenOption.AggressionScore;
	}
}