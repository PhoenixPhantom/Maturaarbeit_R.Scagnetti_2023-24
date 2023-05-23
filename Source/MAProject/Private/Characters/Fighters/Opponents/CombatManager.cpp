// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/CombatManager.h"

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

void ACombatManager::RegisterCombatParticipant(AOpponentCharacter* Participant, FManageCombatParticipantsKey Key)
{
	if(ActiveParticipants.Contains(Participant) || PassiveParticipants.Contains(Participant)) return;
	PassiveParticipants.Add(Participant);
	AttemptDistributeRemainingTokens();
}

void ACombatManager::UnregisterCombatParticipant(AOpponentCharacter* Participant, FManageCombatParticipantsKey Key)
{
	if(AnticipatedActive.Holder == Participant) AnticipatedActive = FAggressionData();
	if(PassiveParticipants.Contains(Participant)) PassiveParticipants.Remove(Participant);
	else ReleaseAggressionToken(Participant, FManageAggressionTokensKey());
}

void ACombatManager::ReleaseAggressionToken(AOpponentCharacter* Participant, FManageAggressionTokensKey Key)
{
	if(!ActiveParticipants.Contains(Participant)) return;
	AvailableAggressionTokens += Participant->GetRequestedTokens();
	ActiveParticipants.Remove(Participant);
	PassiveParticipants.Add(Participant);
	AttemptDistributeRemainingTokens();
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

bool ACombatManager::GrantToken(const FAggressionData& AggressionData)
{
	check(PassiveParticipants.Contains(AggressionData.Holder));
	if(AggressionData.RequestedTokens > AvailableAggressionTokens) return false;
	AvailableAggressionTokens -= AnticipatedActive.RequestedTokens;
	PassiveParticipants.Remove(AggressionData.Holder);
	ActiveParticipants.Add(AggressionData.Holder);
	AggressionData.Holder->OnAggressionTokenGranted.Broadcast();
	return true;
}

void ACombatManager::AttemptDistributeRemainingTokens()
{
	
	if(IsValid(AnticipatedActive.Holder))
	{
		if(!GrantToken(AnticipatedActive)) return;
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
		if(!GrantToken(ChosenOption))
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
	
	/*//We want to know what the best possible state would be, so we can work towards it
	TArray<FAggressionData> BestOptions;
	FindBestToFill(BestOptions, RelevantData, MaxAggressionTokens);

	//We keep granting the best options aggression tokens, until we are out of tokens or the highest scoring option that
	//remains requires to many tokens
	while(AvailableAggressionTokens > 0)
	{
		//Get the best scoring option that remains
		FAggressionData BestOption;
		for(const FAggressionData& Option : BestOptions)
		{
			if(Option.AggressionScore > BestOption.AggressionScore)
				BestOption = Option;
		}
		if(AvailableAggressionTokens >= BestOption.RequestedTokens)
		{
			BestOptions.Remove(BestOption);

			//Grant the aggression token
			PassiveParticipants.Remove(BestOption.Holder);
			AvailableAggressionTokens -= BestOption.RequestedTokens;
			ActiveParticipants.Add(BestOption.Holder);
			BestOption.Holder->OnAggressionTokenGranted.Broadcast();
		}
		//to prevent systematically preferring smaller token sizes,
		//we sometimes have to leave some space free, so larger token sizes have a chance to be used
		else break;
	}*/
}

float ACombatManager::FindBestToFill(TArray<FAggressionData>& BestOptions,
                                     const TArray<FAggressionData>& OptionsToFillWith, uint32 TokenSpaceToFill)
{
	//to optimize performance we can stop the function if we either try to grant tokens to no one or have no tokens to grant
	if(OptionsToFillWith.IsEmpty() || TokenSpaceToFill == 0) return 0.f;
	float BestWeightedScore = 0.f;

	//Get the best target that can use all of the tokens at once (permutation i + 0)
	FAggressionData BestSingle;
	for(const FAggressionData& Data : OptionsToFillWith)
	{
		if(Data.RequestedTokens == TokenSpaceToFill && Data.AggressionScore >= BestSingle.AggressionScore)
		{
			BestSingle = Data;
			BestOptions.Empty();
			BestOptions.Add(BestSingle);
			BestWeightedScore = BestSingle.AggressionScore;
		}
	}
	
	//Cycles through every permutation of two integers that add up to SpaceToFill exactly one time
	//(except to obvious i + 0 which is already taken care of by the code above)
	for(uint32 i = 1; i <= static_cast<uint32>(static_cast<float>(TokenSpaceToFill))/2.f; i++)
	{
		TArray<FAggressionData> FillOptions = OptionsToFillWith;
		
		FAggressionData BestSingleTop;
		FAggressionData BestSingleBottom;
		//Get the two best targets that require exactly the correct amount of tokens
		for(const FAggressionData& Data : FillOptions)
		{
			if(Data.RequestedTokens == TokenSpaceToFill - i && Data.AggressionScore > BestSingleTop.AggressionScore)
				BestSingleTop = Data;
			else if(Data.RequestedTokens == i && Data.AggressionScore > BestSingleBottom.AggressionScore)
				BestSingleBottom = Data;
		}

		//Try to make a group that also uses up the demanded amount of tokens by combining multiple smaller ones (it is
		//possible that a group doesn't use up all the tokens. This is accounted for in the WeightedScoreTop, which is
		//the total score of the group but weighted by the amount of tokens --> Score1 * Tokens1 + Score2 * Tokens2 + ...)		
		TArray<FAggressionData> BestOptionsTop;
		float WeightedScoreTop = FindBestToFill(BestOptionsTop, FillOptions, TokenSpaceToFill - i);
		if(BestSingleTop.AggressionScore * static_cast<float>(TokenSpaceToFill - i) > WeightedScoreTop * static_cast<float>(TokenSpaceToFill))
		{
			BestOptionsTop = { BestSingleTop };
			WeightedScoreTop = BestSingleTop.AggressionScore * static_cast<float>(TokenSpaceToFill - i) / static_cast<float>(TokenSpaceToFill);
		}
		
		for(const FAggressionData& Data : BestOptionsTop) FillOptions.Remove(Data);

		//Try to make a group that also uses up the demanded amount of tokens by combining multiple smaller ones
		TArray<FAggressionData> BestOptionsBottom;
		float WeightedScoreBottom = FindBestToFill(BestOptionsBottom, FillOptions, i);
		check(BestOptionsBottom.IsEmpty());
		if(BestSingleBottom.AggressionScore * static_cast<float>(i) > WeightedScoreBottom * static_cast<float>(TokenSpaceToFill))
		{
			BestOptionsBottom = { BestSingleBottom };
			WeightedScoreBottom = BestSingleTop.AggressionScore * static_cast<float>(i) / static_cast<float>(TokenSpaceToFill);
		}

		const float WeightedScore = WeightedScoreTop + WeightedScoreBottom;
		if(WeightedScore >= BestWeightedScore)
		{
			BestOptions.Empty();
			BestOptions.Append(BestOptionsTop);
			BestOptions.Append(BestOptionsBottom);
			BestWeightedScore = WeightedScore;
			//If we can grant everyone a token anyways, we don't have to continue calculating
			if(BestOptions == OptionsToFillWith) break;
		}
	}
	return BestWeightedScore/static_cast<float>(TokenSpaceToFill);
}
