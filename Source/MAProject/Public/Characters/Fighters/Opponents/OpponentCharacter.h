// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include <random>

#include "CoreMinimal.h"
#include "Characters/Fighters/FighterCharacter.h"
#include "Utility/NonPlayerFunctionality/PositionalConstraint.h"
#include "Utility/Tools/pcg-cpp/include/pcg_random.hpp"
#include "OpponentCharacter.generated.h"

class UBlackboardComponent;
class UBTTask_ExecuteAttackTask;
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

struct FClearCombatTargetKey final
{
	friend ACombatManager;
private:
	FClearCombatTargetKey(){};
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
	friend UBTTask_ExecuteAttackTask;
private:
	FExecuteOnAggressionTokensReleasedKey(){}
};

struct FExecuteAttackKey final
{
	friend UBTTask_ExecuteAttackTask;
private:
	FExecuteAttackKey(){}
};

struct FRequestAttackKey final
{
	friend ACombatManager;
private:
	FRequestAttackKey(){}
};

struct FClearRequestedAttackKey final
{
	friend UBTTask_ExecuteAttackTask;
private:
	FClearRequestedAttackKey(){}
};

struct FSetUsedBlackboardKey final
{
	friend AOpponentController;
private:
	FSetUsedBlackboardKey(){}
};

struct FResetOpponentStatsKey final
{
	friend ACombatManager;
private:
	FResetOpponentStatsKey(){}
};

/**
 * 
 */
UCLASS()
class MAPROJECT_API AOpponentCharacter : public AFighterCharacter
{
	GENERATED_BODY()
public:
	//MoveTo calculates distance differently from us, so we need some margin of error for our distance calculations
	static constexpr float MoveToDistanceMarginOfError = 15.f;
	
	inline static FName RequiredSpaceActiveTag = "RSActive";
	
	AOpponentCharacter(const FObjectInitializer& ObjectInitializer);
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual float GetFieldOfView() const override { return LocalFieldOfView; }
	
	virtual FGenericTeamId GetGenericTeamId() const override { return 1; }

	void BindOnAggressionTokensGranted(const TDelegate<void()>& FunctionToBind, FEditOnAggressionTokensGrantedOrReleasedKey);
	void BindOnAggressionTokensReleased(const TDelegate<void()>& FunctionToBind, FEditOnAggressionTokensGrantedOrReleasedKey);
	
	void ExecuteOnAggressionTokensGranted(FExecuteOnAggressionTokensGrantedKey) const;
	void ExecuteOnAggressionTokensReleased(FExecuteOnAggressionTokensReleasedKey) const;
	void ResetAllStats(FResetOpponentStatsKey) const { CharacterStats->Reset(); }

	FRequiredSpace GetRequiredSpace() const;
	USphereComponent* GetRequiredSpaceActive() const;

	FCircularDistanceConstraint GetActivePlayerDistanceConstraint() const;
	const FCircularDistanceConstraint& GetPassivePlayerDistanceConstraint() const{ return DistanceFromTargetPassive; }
	
	uint32 GetRequestedTokens() const { return RequestedAggressionTokens; }
	UAdvancedCharacterMovementComponent* GetAdvancedCharacterMovement() const{ return AdvancedCharacterMovementComponent; }
	UCharacterRotationManagerComponent* GetCharacterRotationManager() const { return RotationManagerComponent; }
	AController* GetCombatTargetController() const;
	ACharacter* GetCombatTarget() const;
	void ClearCombatTarget(FClearCombatTargetKey);
	void RegisterCombatTarget(AController* NewOpponent, FSetCombatTargetKey);
	void SetLocalFieldOfView(float FieldOfView, FSetFieldOfViewKey){ LocalFieldOfView = FieldOfView; }
	UBlackboardComponent* GetUsedBlackboardComponent() const { return UsedBlackboardComponent; }
	void SetUsedBlackboardComponent(UBlackboardComponent* NewBlackboard, FSetUsedBlackboardKey);

	FORCEINLINE bool ExecuteAttackFromNode(UAttackNode* NodeToExecute, FExecuteAttackKey) const;
	
	void SetRequestedAttack(UAttackNode* NewRequestedAttack, FRequestAttackKey){ RequestedAttack = NewRequestedAttack; }
	void ClearRequestedAttack(FClearRequestedAttackKey){ RequestedAttack = nullptr; }
	UAttackNode* GetRequestedAttack() const;
	UAttackNode* GetRandomValidAttack() const;
	UAttackNode* GetRandomValidAttackInRange() const;

	/**
	 * @brief Generate a score representing the importance of the opponent to the given player. Screen centered-ness,
	 * distance from player and opponent preferences as well as opponent importance are taken into account
	 * @param PlayerCharacter The player in regards to which the score is generated
	 * @return the score is always >= 0.f if the opponent is allowed to become aggressive (normally: <= 4.5f;
	 * if character is the player's target: <= 8.5f)*/
	 float GenerateAggressionScore(APlayerCharacter* PlayerCharacter) const;

protected:
	inline static pcg_extras::seed_seq_from<std::random_device> SeedSource;
	inline static pcg32 RandomGenerator = SeedSource;
	uint8 bCanBecomeAggressive:1;
	float LocalFieldOfView;

	
	TDelegate<void()> OnAggressionTokensGranted;
	TDelegate<void()> OnAggressionTokensRemoved;

	UPROPERTY()
	UAttackNode* RequestedAttack;
	
	UPROPERTY()
	AController* TargetPlayer;

	UPROPERTY()
	UBlackboardComponent* UsedBlackboardComponent;
	
	UPROPERTY(SaveGame)
	FSavableCharacterModifiers StatsModifiers;

	UPROPERTY(EditAnywhere)
	UAdvancedCharacterMovementComponent* AdvancedCharacterMovementComponent;
	
	UPROPERTY(EditAnywhere)
	UCharacterRotationManagerComponent* RotationManagerComponent;

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
	
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ToughnessBrokenAnimation;
	
	UPROPERTY(EditAnywhere, Category=AI)
	uint32 RequestedAggressionTokens;
	
	UPROPERTY(EditAnywhere, Category=AI)
	float AggressionPriority;

	UPROPERTY(EditAnywhere, Category=AI, AdvancedDisplay)
	float AggressionRange;

	virtual void BeginPlay() override;
	virtual bool TriggerDeath() override;
	virtual void GetStaggered(bool HeavyStagger) override;
	virtual bool TriggerToughnessBroken() override;
	virtual void RestoreToughness() override;
	virtual void OnGetAttacked(const FAttackDamageEvent* DamageEvent) override;

	bool CanAttack() const{ return CanAttackInSeconds() <= 0.f; };
	float CanAttackInSeconds() const;
	static float EarliestAttackSeconds(const UGenericGraphNode* SourceNode);
	
	void SetUseActiveCombatSpace() const;
	void SetUsePassiveSpace() const;

	UFUNCTION()
	void OnAttackTreeRootChanged(){ RequestedAttack = nullptr; };
	
	UFUNCTION()
	void OnSelectMotionWarpingTarget(const FAttackProperties& Properties);	
};