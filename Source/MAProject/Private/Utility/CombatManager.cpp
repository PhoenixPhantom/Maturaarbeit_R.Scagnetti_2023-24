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
ACombatManager::ACombatManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

FVector ACombatManager::GetAggressivenessDependantLocation(AOpponentCharacter* OwningCharacter)
{
	if(!IsParticipant(OwningCharacter))
	{
		checkNoEntry();
		return FVector(NAN);
	}
	const FVector OpponentLocation = OwningCharacter->GetActorLocation();

	
	TArray<FPositionalConstraint> PositionalConstraints;
	GetPositionalConstraints(PositionalConstraints);
	
	FPlayerDistanceConstraint PlayerDistanceConstraint;
	if(ActiveParticipants.Contains(OwningCharacter)) PlayerDistanceConstraint = OwningCharacter->GetActivePlayerDistanceConstraint();
	else PlayerDistanceConstraint = OwningCharacter->GetPassivePlayerDistanceConstraint();
	PlayerDistanceConstraint.Player = PlayerCharacter;
	PositionalConstraints.Add(PlayerDistanceConstraint);

	FPlayerRelativeWorldZoneConstraint ZoneConstraint(PlayerCharacter);
	ZoneConstraint.ConstraintZone = ZoneConstraint.CalculateTargetZone(OpponentLocation);
	PositionalConstraints.Add(ZoneConstraint);

	//We only have to expensively find a new location if the current one isn't good anymore
	bool AreAllSatisfied = true;
	for(const FPositionalConstraint& Constraint : PositionalConstraints)
	{
		if(!Constraint.IsConstraintSatisfied(OpponentLocation))
		{
			AreAllSatisfied = false;
			break;
		}
	}
	if(AreAllSatisfied) return OpponentLocation;
	
	FVector Location;
	FRotator Rotation;
	OwningCharacter->GetActorEyesViewPoint(Location, Rotation);
	return SampleGetClosestValid(OwningCharacter->GetActorLocation(), Rotation.Vector() * 100.f,
		50.f, PositionalConstraints);
}

void ACombatManager::GetPositionalConstraints(TArray<FPositionalConstraint>& PositionalConstraints,
                                              const AOpponentCharacter* Excepted)
{
	for(const AOpponentCharacter* Passive : PassiveParticipants)
	{
		if(Passive == Excepted) continue;
		FPassiveCombatConstraint PassiveCombatConstraint = Passive->GetPassivePositionConstraint();
		PassiveCombatConstraint.OrientationCenter = PlayerCharacter;
		PositionalConstraints.Add(PassiveCombatConstraint);
	}
	for(const AOpponentCharacter* Active : ActiveParticipants)
	{
		if(Active == Excepted) continue;
		PositionalConstraints.Add(Active->GetActivePositionConstraint());
	}
}

bool ACombatManager::IsParticipant(AFighterCharacter* Character) const
{
	if(Character == PlayerCharacter) return true;
	if(PassiveParticipants.Contains(Character)) return true;
	return ActiveParticipants.Contains(Character);
}

void ACombatManager::RegisterCombatParticipant(APlayerCharacter* PlayerParticipant, FManageCombatParticipantsKey Key)
{
	PlayerCharacter = PlayerParticipant;
}

bool ACombatManager::RegisterCombatParticipant(AOpponentCharacter* Participant, FManageCombatParticipantsKey Key)
{
	if(IsParticipant(Participant)) return false;
	PassiveParticipants.Add(Participant);
	AttemptDistributeRemainingTokens();
	return true;
}

void ACombatManager::UnregisterCombatParticipant(AOpponentCharacter* Participant, FManageCombatParticipantsKey Key)
{
	if(AnticipatedActive.Holder == Participant) AnticipatedActive = FAggressionData();
	RemoveAggressionTokens(Participant);
	PassiveParticipants.Remove(Participant);
}

void ACombatManager::ReleaseAggressionTokens(AOpponentCharacter* Participant, FManageAggressionTokensKey Key)
{
	if(RemoveAggressionTokens(Participant))	AttemptDistributeRemainingTokens();
}

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
	AggressionData.Holder->OnAggressionTokensGranted.Broadcast();
	return true;
}

bool ACombatManager::MakeActiveParticipant(int32 Index)
{
	if(!PassiveParticipants.IsValidIndex(Index))
	{
		checkNoEntry();
		return false;
	}
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
	PassiveParticipants.Add(ActiveParticipants[Index]);
	ActiveParticipants.RemoveAt(Index);
	return true;
}

void ACombatManager::AttemptDistributeRemainingTokens()
{
	constexpr float PreferBestScorePower = 4.f;
	if(IsValid(AnticipatedActive.Holder))
	{
		if(!GrantTokens(AnticipatedActive)) return;
	}
	
	//Generate the aggression score for every passive participant
	double TotalAggressionScoreSquared = 0.0;
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
			TotalAggressionScoreSquared += pow(Score, PreferBestScorePower);
			
			GLog->Log(FString::SanitizeFloat(Score));
			RelevantData.Add(FAggressionData(Score, Participant, RequestedTokens));
		}
	}
	
	while(AvailableAggressionTokens > 0 && !RelevantData.IsEmpty())
	{
		FAggressionData ChosenOption;
		//we use double in the assignment because we could be dealing with very small numbers and we still want high accuracy
		float RandomNumber = static_cast<double>(rand()) * TotalAggressionScoreSquared / static_cast<double>(RAND_MAX);
		for(const FAggressionData& AggressionData : RelevantData)
		{
			RandomNumber -= pow(AggressionData.AggressionScore, PreferBestScorePower);
			if(RandomNumber <= 0.f)
			{
				
				GLog->Log("Chose:" + FString::SanitizeFloat(AggressionData.AggressionScore));
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
		TotalAggressionScoreSquared -= pow(ChosenOption.AggressionScore, PreferBestScorePower);
	}
	GLog->Log("Ended");
}
