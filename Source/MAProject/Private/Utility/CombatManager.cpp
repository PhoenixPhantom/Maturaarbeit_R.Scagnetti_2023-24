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

void ACombatManager::RegisterCombatParticipant(APlayerCharacter* PlayerParticipant, FManageCombatParticipantsKey Key)
{
	PlayerCharacter = PlayerParticipant;
}

bool ACombatManager::RegisterCombatParticipant(AOpponentCharacter* Participant, FManageCombatParticipantsKey Key)
{
	if(ActiveParticipants.Contains(Participant) || PassiveParticipants.Contains(Participant)) return false;
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
	ActiveParticipants.Remove(Participant);
	PassiveParticipants.Add(Participant);
	return true;
}

bool ACombatManager::GrantTokens(const FAggressionData& AggressionData)
{
	check(PassiveParticipants.Contains(AggressionData.Holder));
	if(AggressionData.RequestedTokens > AvailableAggressionTokens) return false;
	AvailableAggressionTokens -= AnticipatedActive.RequestedTokens;
	PassiveParticipants.Remove(AggressionData.Holder);
	ActiveParticipants.Add(AggressionData.Holder);
	AggressionData.Holder->OnAggressionTokensGranted.Broadcast();
	return true;
}

void ACombatManager::AttemptDistributeRemainingTokens()
{
	
	if(IsValid(AnticipatedActive.Holder))
	{
		if(!GrantTokens(AnticipatedActive)) return;
	}
	
	//Generate the aggression score for every passive participant
	double TotalAggressionScore = 0.0;
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
			TotalAggressionScore += Score;
			RelevantData.Add(FAggressionData(Score, Participant, RequestedTokens));
		}
	}
	
	while(AvailableAggressionTokens > 0)
	{
		FAggressionData ChosenOption;
		//we use double in the assignment because we could be dealing with very small numbers and we still want high accuracy
		float RandomNumber = static_cast<double>(rand()) / static_cast<double>(RAND_MAX) * TotalAggressionScore;
		for(const FAggressionData& AggressionData : RelevantData)
		{
			RandomNumber -= AggressionData.AggressionScore;
			if(RandomNumber <= 0.f)
			{
				ChosenOption = AggressionData;
				break;
			}
		}
		if(!IsValid(ChosenOption.Holder)) break;
		if(!GrantTokens(ChosenOption))
		{
			//if the entity cannot be granted enough tokens and other entities are can't get a token because of
			//the chosen option the entity should be guaranteed to be the next one to be granted tokens (it is anticipated)
			if(RelevantData.Num() > 1) AnticipatedActive = ChosenOption;
			break;
		}
		//if we were able to grant the token, the entity cannot be granted a token again until it releases it
		RelevantData.Remove(ChosenOption);
		TotalAggressionScore -= ChosenOption.AggressionScore;
	}
}