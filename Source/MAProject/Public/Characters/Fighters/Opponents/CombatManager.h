// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CombatManager.generated.h"


class AOpponentCharacter;
class APlayerCharacter;
class APlayerPartyController;
class AOpponentController;

struct FManageCombatParticipantsKey final
{
	friend AOpponentController;
	friend APlayerPartyController;
private:
	FManageCombatParticipantsKey(){};
};

struct FManageAggressionTokensKey final
{
	friend class ACombatManager;
private:
	FManageAggressionTokensKey(){};
};

struct FAggressionData
{
	FAggressionData() : AggressionScore(std::numeric_limits<float>::lowest()), Holder(nullptr), RequestedTokens(0) {}
	FAggressionData(float NewScore, uint32 NewToken) : FAggressionData(NewScore, nullptr, NewToken){}
	FAggressionData(float NewScore, AOpponentCharacter* NewHolder, uint32 NewToken) : AggressionScore(NewScore), Holder(NewHolder),
		RequestedTokens(NewToken){}
	FAggressionData(const FAggressionData& AggressionData) = default;
	
	float AggressionScore;
	AOpponentCharacter* Holder;
	uint32 RequestedTokens;

	bool operator==(const FAggressionData& AggressionData) const;


	float GetScorePerToken() const { return AggressionScore/static_cast<float>(RequestedTokens); }
};

UCLASS()
class MAPROJECT_API ACombatManager : public AActor
{
	GENERATED_BODY()

public:
	ACombatManager();

	APlayerCharacter* GetPlayerCharacter() const { return PlayerCharacter; };
	
	void RegisterCombatParticipant(APlayerCharacter* PlayerParticipant, FManageCombatParticipantsKey Key);
	void RegisterCombatParticipant(AOpponentCharacter* Participant, FManageCombatParticipantsKey Key);
	void UnregisterCombatParticipant(AOpponentCharacter* Participant, FManageCombatParticipantsKey Key);

	void ReleaseAggressionToken(AOpponentCharacter* Participant, FManageAggressionTokensKey Key);

protected:
	TArray<FAggressionData> ActiveRequests;
	uint32 AvailableAggressionTokens;
	
	UPROPERTY()
	APlayerCharacter* PlayerCharacter;

	UPROPERTY()
	TArray<AOpponentCharacter*> ActiveParticipants;
	UPROPERTY()
	TArray<AOpponentCharacter*> PassiveParticipants;

	UPROPERTY(EditAnywhere)
	uint32 MaxAggressionTokens;
	
	virtual void BeginPlay() override;

	//Try to distribute the AvailableAggressionTokens so the highest scoring objects will be inserted
	void AttemptDistributeRemainingTokens();

	/**
	 * @brief Calculates the best possible combination of options that uses no more than the given amount of tokens
	 * @param BestOptions Array to where the best combination of options is written
	 * @param OptionsToFillWith the possible options from which the best options are chosen
	 * @param TokenSpaceToFill The amount of tokens that the combination can use (if appropriate, some tokens will not be used)
	 * @return The weighted average AggressionScore of the combination considered optimal, using RequestedTokens as weight*/
	static float FindBestToFill(TArray<FAggressionData>& BestOptions, const TArray<FAggressionData>& OptionsToFillWith,
	                            uint32 TokenSpaceToFill);
};
