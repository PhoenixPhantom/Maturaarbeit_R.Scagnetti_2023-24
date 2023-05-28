// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CombatManager.generated.h"


class AFighterCharacter;
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
	friend class AOpponentController;
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

	APlayerCharacter* GetPlayerCharacter() const { return PlayerCharacter; }
	FVector GetAggressivenessDependantLocation(AOpponentCharacter* OwningCharacter);
	bool IsParticipant(AFighterCharacter* Character) const;
	
	void RegisterCombatParticipant(APlayerCharacter* PlayerParticipant, FManageCombatParticipantsKey Key);
	bool RegisterCombatParticipant(AOpponentCharacter* Participant, FManageCombatParticipantsKey Key);
	void UnregisterCombatParticipant(AOpponentCharacter* Participant, FManageCombatParticipantsKey Key);

	void ReleaseAggressionTokens(AOpponentCharacter* Participant, FManageAggressionTokensKey Key);

protected:
	TArray<FAggressionData> ActiveRequests;
	uint32 AvailableAggressionTokens;
	
	
	FAggressionData AnticipatedActive;
	
	UPROPERTY()
	APlayerCharacter* PlayerCharacter;
	
	UPROPERTY()
	TArray<AOpponentCharacter*> ActiveParticipants;
	UPROPERTY()
	TArray<AOpponentCharacter*> PassiveParticipants;

	UPROPERTY(EditAnywhere)
	uint32 MaxAggressionTokens;
	
	virtual void BeginPlay() override;

	bool RemoveAggressionTokens(AOpponentCharacter* Participant);

	//Grants the requested number of tokens, if enough are available. If not, returns false.
	bool GrantTokens(const FAggressionData& AggressionData);

	bool MakeActiveParticipant(int32 Index);
	bool MakePassiveParticipant(int32 Index);

	//Try to distribute the AvailableAggressionTokens so the highest scoring objects will be inserted
	void AttemptDistributeRemainingTokens();
};
