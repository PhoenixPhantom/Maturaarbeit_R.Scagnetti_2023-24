// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Fighters/FighterCharacter.h"
#include "Characters/Fighters/Player/PlayerCharacter.h"
#include "Utility/NonPlayerFunctionality/PositionalConstraint.h"
#include "OpponentCharacter.generated.h"

class UPatrolManagerComponent;
class UPlayerFacingWidgetComponent;
class UWidgetComponent;
class UBTTask_ReleaseAggressionTokens;
class ACombatManager;
class USphereComponent;
class UBoxComponent;
class AOpponentController;
class USavableObjectMarkerComponent;
class UTargetInformationComponent;
class UAdvancedCharacterMovementComponent;
class UCharacterRotationManagerComponent;

struct FSetFieldOfViewKey final
{
	friend AOpponentController;
private:
	FSetFieldOfViewKey(){};
};

struct FSetCombatTargetKey final
{
	friend AOpponentController;
private:
	FSetCombatTargetKey(){};
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
	friend UBTTask_ReleaseAggressionTokens;
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
	
	AOpponentCharacter(const FObjectInitializer& ObjectInitializer);
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual float GetFieldOfView() const override { return LocalFieldOfView; }
	
	virtual FGenericTeamId GetGenericTeamId() const override { return 1; }
	
	FOnAggressionTokenGrantedDelegate& GetOnAggressionTokensGranted(FEditOnAggressionTokensGrantedOrReleasedKey)
		{ return OnAggressionTokensGranted; }
	FOnAggressionTokenRemovedDelegate& GetOnAggressionTokensReleased(FEditOnAggressionTokensGrantedOrReleasedKey)
		{ return OnAggressionTokensRemoved; }
	void ExecuteOnAggressionTokensGranted(FExecuteOnAggressionTokensGrantedKey) const
		{ OnAggressionTokensGranted.Broadcast(); }
	void ExecuteOnAggressionTokensReleased(FExecuteOnAggressionTokensReleasedKey) const
		{ OnAggressionTokensRemoved.Broadcast(); }

	UShapeComponent* GetRequiredSpace() const;

	FCircularDistanceConstraint GetActivePlayerDistanceConstraint() const;
	const FCircularDistanceConstraint& GetPassivePlayerDistanceConstraint() const{ return DistanceFromTargetPassive; };
	
	uint32 GetRequestedTokens() const { return RequestedAggressionTokens; }
	AActor* GetTargetPlayer() const { return IsValid(TargetPlayer) ? TargetPlayer->GetPawn() : nullptr; }
	UAdvancedCharacterMovementComponent* GetAdvancedCharacterMovement() const{ return AdvancedCharacterMovementComponent; }
	UCharacterRotationManagerComponent* GetCharacterRotationManager() const { return RotationManagerComponent; }
	AController* GetCombatTargetController() const;
	ACharacter* GetCombatTarget() const;
	void RegisterCombatTarget(AController* NewOpponent, FSetCombatTargetKey Key);
	void SetLocalFieldOfView(float FieldOfView, FSetFieldOfViewKey Key){ LocalFieldOfView = FieldOfView; }

	/**
	 * @brief Generate a score for the importance of the opponent to the player. Screen centered-ness,
	 * distance from player and opponent preferences as well as opponent importance are taken into account
	 * @param PlayerCharacter The player in regards to which the score is generated
	 * @return the score is always >= 0.f if the opponent is allowed to become aggressive*/
	 float GenerateAggressionScore(APlayerCharacter* PlayerCharacter) const;

protected:
	uint8 bCanBecomeAggressive:1;
	float LocalFieldOfView;
	
	FOnAggressionTokenGrantedDelegate OnAggressionTokensGranted;
	FOnAggressionTokenRemovedDelegate OnAggressionTokensRemoved;

	UPROPERTY()
	UAdvancedCharacterMovementComponent* AdvancedCharacterMovementComponent;

	UPROPERTY()
	UCharacterRotationManagerComponent* RotationManagerComponent;


	UPROPERTY()
	AController* TargetPlayer;
	
	UPROPERTY(SaveGame)
	FSavableCharacterModifiers StatsModifiers;

	UPROPERTY(EditAnywhere, Category = UserInterface)
	UPlayerFacingWidgetComponent* HealthWidgetComponent;
	
	UPROPERTY(EditAnywhere, Category = UserInterface)
	UPatrolManagerComponent* PatrolManagerComponent;
	
	UPROPERTY(EditAnywhere)
	USavableObjectMarkerComponent* SavableObjectMarkerComponent;

	UPROPERTY(EditAnywhere)
	USphereComponent* RequiredSpaceActiveCombat;

	UPROPERTY(EditAnywhere)
	UBoxComponent* RequiredSpacePassive;

	UPROPERTY(EditAnywhere, Category=Combat)
	FCircularDistanceConstraint DistanceFromTargetPassive;
	
	UPROPERTY(EditAnywhere, Category=AI)
	uint32 RequestedAggressionTokens;
	
	UPROPERTY(EditAnywhere, Category=AI)
	float AggressionPriority;

	UPROPERTY(EditAnywhere, Category=AI, AdvancedDisplay)
	float AggressionRange;

	virtual void BeginPlay() override;
	virtual void OnDeathTriggered() override;;

	bool CanAttack() const{ return CanAttackInSeconds() <= 0.f; };
	float CanAttackInSeconds() const;


	UFUNCTION()
	void SetUseActiveCombatSpace();

	UFUNCTION()
	void SetUsePassiveSpace();
	
	UFUNCTION()
	void OnSelectMotionWarpingTarget(const FAttackProperties& Properties);	
};