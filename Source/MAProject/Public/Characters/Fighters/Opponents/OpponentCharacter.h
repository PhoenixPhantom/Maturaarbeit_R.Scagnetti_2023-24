// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Fighters/FighterCharacter.h"
#include "Characters/Fighters/Player/PlayerCharacter.h"
#include "Utility/NonPlayerFunctionality/PositionalConstraint.h"
#include "OpponentCharacter.generated.h"

class UReleaseAggressionTokensTask;
class ACombatManager;
class USphereComponent;
class UBoxComponent;
class AOpponentController;
class USavableObjectMarkerComponent;
class UTargetInformationComponent;

struct FSetFieldOfViewKey final
{
	friend AOpponentController;
private:
	FSetFieldOfViewKey(){};
};

struct FSetPlayerOpponentKey final
{
	friend AOpponentController;
private:
	FSetPlayerOpponentKey(){};
};

struct FEditOnAggressionTokensGrantedOrReleasedKey final
{
	friend AOpponentController;
private:
	FEditOnAggressionTokensGrantedOrReleasedKey(){}
};

struct FEditOnOpponentDespawnedKey final
{
	friend AOpponentController;
private:
	FEditOnOpponentDespawnedKey(){}
};

struct FExecuteOnAggressionTokensGrantedKey final
{
	friend ACombatManager;
private:
	FExecuteOnAggressionTokensGrantedKey(){}
};

struct FExecuteOnAggressionTokensReleasedKey final
{
	friend UReleaseAggressionTokensTask;
private:
	FExecuteOnAggressionTokensReleasedKey(){}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAggressionTokenGrantedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAggressionTokenRemovedDelegate);

/**
 * 
 */
UCLASS()
class MAPROJECT_API AOpponentCharacter : public AFighterCharacter
{
	GENERATED_BODY()
public:
	inline static FName RequiredSpaceActiveTag = "RSActive";
	
	AOpponentCharacter();
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual float GetFieldOfView() const override { return LocalFieldOfView; }
	
	FOnAggressionTokenGrantedDelegate& GetOnAggressionTokensGranted(FEditOnAggressionTokensGrantedOrReleasedKey)
		{ return OnAggressionTokensGranted; }
	FOnAggressionTokenRemovedDelegate& GetOnAggressionTokensReleased(FEditOnAggressionTokensGrantedOrReleasedKey)
		{ return OnAggressionTokensRemoved; }
	void ExecuteOnAggressionTokensGranted(FExecuteOnAggressionTokensGrantedKey) const
		{ OnAggressionTokensGranted.Broadcast(); }
	void ExecuteOnAggressionTokensReleased(FExecuteOnAggressionTokensReleasedKey) const
		{ OnAggressionTokensRemoved.Broadcast(); }

	UShapeComponent* GetRequiredSpace() const;
	
	const FPlayerDistanceConstraint* GetActivePlayerDistanceConstraint() const { return &DistanceFromTargetActive; };
	const FPlayerDistanceConstraint* GetPassivePlayerDistanceConstraint() const{ return &DistanceFromTargetPassive; };
	
	uint32 GetRequestedTokens() const { return RequestedAggressionTokens; }
	void RegisterPlayerOpponent(AController* NewOpponent, FSetPlayerOpponentKey Key);
	void SetLocalFieldOfView(float FieldOfView, FSetFieldOfViewKey Key){ LocalFieldOfView = FieldOfView; }

	/**
	 * @brief Generate a score for the importance of the opponent to the player. Screen centered-ness,
	 * distance from player and opponent preferences as well as opponent importance are taken into account
	 * @param PlayerCharacter The player in regards to which the score is generated
	 * @return the score >= 0.f if the opponent is allowed to become aggressive*/
	 float GenerateAggressionScore(APlayerCharacter* PlayerCharacter) const;

protected:
	uint8 bCanBecomeAggressive:1;
	float LocalFieldOfView;

	FOnAggressionTokenGrantedDelegate OnAggressionTokensGranted;
	FOnAggressionTokenRemovedDelegate OnAggressionTokensRemoved;


	UPROPERTY()
	AController* TargetPlayer;
	
	UPROPERTY(SaveGame)
	FSavableCharacterModifiers StatsModifiers;
	
	UPROPERTY(EditAnywhere)
	USavableObjectMarkerComponent* SavableObjectMarkerComponent;

	UPROPERTY(EditAnywhere)
	USphereComponent* RequiredSpaceActiveCombat;

	UPROPERTY(EditAnywhere)
	UBoxComponent* RequiredSpacePassive;

	UPROPERTY(EditAnywhere, Category=Combat)
	FPlayerDistanceConstraint DistanceFromTargetActive;

	UPROPERTY(EditAnywhere, Category=Combat)
	FPlayerDistanceConstraint DistanceFromTargetPassive;
	
	UPROPERTY(EditAnywhere, Category=AI)
	uint32 RequestedAggressionTokens;
	
	UPROPERTY(EditAnywhere, Category=AI)
	float AggressionPriority;

	UPROPERTY(EditAnywhere, Category=AI, AdvancedDisplay)
	float AggressionRange;

	virtual void BeginPlay() override;


	UFUNCTION()
	void SetUseActiveCombatSpace();

	UFUNCTION()
	void SetUsePassiveSpace();
	
	UFUNCTION()
	void OnSelectMotionWarpingTarget(const FAttackProperties& Properties);	
};