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
ACombatManager::ACombatManager() : PlayerCharacter(nullptr), MaxAggressionTokens(2),
	AvailableAggressionTokens(MaxAggressionTokens), PreferBestScorePower(4.f)
{
#if WITH_EDITORONLY_DATA
	PrimaryActorTick.bCanEverTick = true;
#else
	PrimaryActorTick.bCanEverTick = false;
#endif
}

ECombatParticipantStatus ACombatManager::GetParticipationStatus(AFighterCharacter* Character) const
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
	if(ECombatParticipantStatus::NotRegistered != GetParticipationStatus(Participant)) return false;
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
	if(bIsDebugging)
	{
		UKismetSystemLibrary::DrawDebugCircle(GetWorld(), Participant->GetActorLocation() +
			FVector(0.f, 0.f, 50.f), 100.f,
		50, FLinearColor(0, 255, 0), 1.f, 10.f, FVector(0, 1, 0),
		FVector(1, 0, 0));
	}
	return MakePassiveParticipant(ActiveParticipants.Find(Participant));
}

bool ACombatManager::GrantTokens(const FAggressionData& AggressionData)
{
	if(!PassiveParticipants.Contains(AggressionData.Holder) || ActiveParticipants.Contains(AggressionData.Holder))
	{
		//non-passive (active) participants cannot be granted a token 
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
		//There is no use trying to distribute tokens to either an entity that needs more than what can maximally be provided
		//or to an entity that doesn't need tokens
		if(RequestedTokens > MaxAggressionTokens || RequestedTokens == 0) continue;
		const float Score = Participant->GenerateAggressionScore(PlayerCharacter);
		//A score < 0.f means that the given participant cannot become aggressive, so it is not relevant
		if(Score >= 0.f)
		{
			if(LowestScore > Score) LowestScore = Score;
			RelevantData.Add(FAggressionData(Score, Participant, RequestedTokens));
		}
	}
	double TotalScoreAdjusted = 0.0;
	for(FAggressionData& Data : RelevantData)
	{
		const double ScoreAdjusted = pow(Data.AggressionScore - LowestScore, PreferBestScorePower);
		TotalScoreAdjusted += ScoreAdjusted;
		//Save the adjusted score so we have the calculation in only one place
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
			if(AvailableAggressionTokens >= 1 && RelevantData.Num() >= 2)
			{
				//if an entity requires more tokens than what is left over, and there are still other entities that might
				//be satisfied with the available amount of tokens, we queue the entity (to be guaranteed to be the
				//next one to receive a token) before stopping token distribution
				//(this should prevent entities requiring a high amount of tokens from never being able to get active,
				//but obviously brings with it some other problems)
				AnticipatedActive = ChosenOption;
			}
			
			break;
		}
		
		//An entity cannot be granted a token twice
		RelevantData.Remove(ChosenOption);
		TotalScoreAdjusted -= ChosenOption.AggressionScore;
	}
}