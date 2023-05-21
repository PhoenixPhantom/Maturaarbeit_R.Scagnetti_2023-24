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

	//There can only be one combat manager at any given time to prevent logic problems
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACombatManager::StaticClass(), Actors);
	check(Actors.Num() == 1 && Actors[0] == this);
}

void ACombatManager::AttemptDistributeRemainingTokens()
{
	//Generate the aggression score for every passive participant
	TArray<FAggressionData> RelevantData;
	for(AOpponentCharacter* Participant : PassiveParticipants)
	{
		RelevantData.Add(FAggressionData(Participant->GenerateAggressionScore(PlayerCharacter), Participant,
			Participant->GetRequestedTokens()));
	}

	//We want to know what the best possible state would be, so we can work towards it
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
	}
}

float ACombatManager::FindBestToFill(TArray<FAggressionData>& BestOptions,
                                     const TArray<FAggressionData>& OptionsToFillWith, uint32 TokenSpaceToFill)
{
	float BestWeightedScore = 0.f;
	//Cycles through every possible combination of two values that add up to SpaceToFill exactly one time
	for(uint32 i = 0; i < std::ceil(static_cast<float>(TokenSpaceToFill)/2.f); i++)
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
		if(BestSingleTop.AggressionScore * static_cast<float>(TokenSpaceToFill - i) > WeightedScoreTop)
		{
			BestOptionsTop = { BestSingleTop };
			WeightedScoreTop = BestSingleTop.AggressionScore * static_cast<float>(TokenSpaceToFill - i);
		}
		
		for(const FAggressionData& Data : BestOptionsTop) FillOptions.Remove(Data);

		//Try to make a group that also uses up the demanded amount of tokens by combining multiple smaller ones
		TArray<FAggressionData> BestOptionsBottom;
		float WeightedScoreBottom = FindBestToFill(BestOptionsBottom, FillOptions, i);
		if(BestSingleBottom.AggressionScore * static_cast<float>(i) > WeightedScoreBottom)
		{
			BestOptionsBottom = { BestSingleBottom };
			WeightedScoreBottom = BestSingleTop.AggressionScore * static_cast<float>(i);
		}

		const float WeightedScore = WeightedScoreTop + WeightedScoreBottom;
		if(WeightedScore > BestWeightedScore)
		{
			BestOptions.Empty();
			BestOptions.Append(BestOptionsTop);
			BestOptions.Append(BestOptionsBottom);
			BestWeightedScore = WeightedScore;
		}
	}
	return BestWeightedScore;
}
