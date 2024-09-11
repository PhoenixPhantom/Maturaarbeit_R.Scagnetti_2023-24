// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CombatManager.generated.h"


class AGlobalSoundManager;
class UAttackNode;
class AFighterCharacter;
class AOpponentCharacter;
class APlayerCharacter;
class APlayerPartyController;
class AOpponentController;

#if WITH_EDITORONLY_DATA
DECLARE_MULTICAST_DELEGATE(FDrawDebugImagesDelegate);
#endif

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

struct FAggressorInfo
{
	FAggressorInfo() : Aggressor(nullptr),
	                   RequestedAttack(nullptr), RequestedTokens(0)
	{}
	FAggressorInfo(AOpponentCharacter* NewHolder, UAttackNode* AttackTreeNode, uint32 NewTokens) : Aggressor(NewHolder),
		RequestedAttack(AttackTreeNode), RequestedTokens(NewTokens)
	{
	}
	
	AOpponentCharacter* Aggressor;
	UAttackNode* RequestedAttack;
	uint32 RequestedTokens;

	bool operator==(const FAggressorInfo& AggressionData) const;
};

struct FScoredAggressorInfo : public FAggressorInfo
{
	FScoredAggressorInfo() : Score(std::numeric_limits<float>::lowest()){}
	FScoredAggressorInfo(AOpponentCharacter* NewHolder, UAttackNode* AttackTreeNode, float NewScore, uint32 NewTokens);
	float Score;
};

UENUM(BlueprintType)
enum class ECombatParticipantStatus : uint8
{
	NotRegistered UMETA(DisplayName="Not Registered"),
	Active UMETA(DisplayName="Active"),
	Passive UMETA(DisplayName="Passive"),
	Player UMETA(DisplayName="Player")
};

UCLASS()
class MAPROJECT_API ACombatManager : public AActor
{
	GENERATED_BODY()

public:
	ACombatManager();

	APlayerCharacter* GetPlayerCharacter() const { return PlayerCharacter; }
	ECombatParticipantStatus GetParticipationStatus(AFighterCharacter* Character) const;

	const TArray<AOpponentCharacter*>& GetAllActiveParticipants(){ return ActiveParticipants; }
	const TArray<AOpponentCharacter*>& GetAllPassiveParticipants(){ return PassiveParticipants; }

	//register or unregister (nullptr) the player for combat
	void RegisterCombatParticipant(APlayerCharacter* PlayerParticipant, FManageCombatParticipantsKey Key);
	//register a NPC for combat
	bool RegisterCombatParticipant(AOpponentCharacter* Participant, FManageCombatParticipantsKey Key);
	//unregister an NPC from combat
	void UnregisterCombatParticipant(AOpponentCharacter* Participant, bool SetToPending, FManageCombatParticipantsKey Key);

	void ReleaseAggressionTokens(AOpponentCharacter* Participant, FManageAggressionTokensKey Key);

#if WITH_EDITORONLY_DATA
	virtual void Tick(float DeltaSeconds) override;
#endif

protected:
	TArray<FAggressorInfo> ActiveRequests;	
	
	FAggressorInfo AnticipatedActive;
	
	UPROPERTY()
	APlayerCharacter* PlayerCharacter;
	
	UPROPERTY()
	TArray<AOpponentCharacter*> ActiveParticipants;
	UPROPERTY()
	TArray<AOpponentCharacter*> PassiveParticipants;
	UPROPERTY()
	TMap<AOpponentCharacter*, FTimerHandle> PendingOutOfCombat;

	UPROPERTY()
	AGlobalSoundManager* SoundManager;

	UPROPERTY(EditAnywhere)
	float PendingOutOfCombatDuration;
	UPROPERTY(EditAnywhere)
	uint32 MaxAggressionTokens;	
	uint32 AvailableAggressionTokens;

	
	virtual void BeginPlay() override;

	bool RemoveAggressionTokens(AOpponentCharacter* Participant);

	//Grants the requested number of tokens, if enough are available. If not, returns false.
	bool GrantTokens(const FAggressorInfo& AggressorInfo);

	bool MakeActiveParticipant(int32 Index);
	bool MakePassiveParticipant(int32 Index);
	static float GetAttackValue(UAttackNode* AttackNode, AOpponentCharacter* Attacker);

	//Try to distribute the AvailableAggressionTokens so the highest scoring objects will be inserted
	void AttemptDistributeFreeTokens();

	void FullyExitFromCombat(AOpponentCharacter* OpponentCharacter);

	UFUNCTION()
	void OnOutOfCombat(AOpponentCharacter* Participant);

#if WITH_EDITORONLY_DATA
	TTuple<float, FDrawDebugImagesDelegate> DebugImagesToDraw;

	UPROPERTY(VisibleAnywhere, Category=Debugging)
	bool bIsDebugging = false;
	UFUNCTION(CallInEditor, Category = Debugging)
	void ToggleDebugging(){ bIsDebugging = !bIsDebugging; };
#endif
};
