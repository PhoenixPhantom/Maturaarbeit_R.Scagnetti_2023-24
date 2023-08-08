// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterStats.h"
#include "Characters/GeneralCharacter.h"
#include "FighterCharacter.generated.h"

class UBoneSoundResponseConfig;
class UTargetInformationComponent;
class UNiagaraSystem;

struct FMeleeControlsKey final
{
	friend class UAnimNotifyState_MeleeAttack;
private:
	FMeleeControlsKey(){}
};

/**
 * 
 */
UCLASS()
class AFighterCharacter : public AGeneralCharacter
{
	GENERATED_BODY()

public:
	AFighterCharacter();

	virtual void Tick(float DeltaSeconds) override;
	
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
		AActor* DamageCauser) override;

	virtual void StopAnimMontage(UAnimMontage* AnimMontage = nullptr) override;

	bool IsMovingOnFloor() const;
	bool IsWalking() const;
	bool IsRunning() const;
	
	void ActivateMeleeBones(const TArray<FName>& BonesToEnable, bool StartEmpty, bool AllowHitRecentVictims,
		FMeleeControlsKey Key);
	void DeactivateMeleeBones(const TArray<FName>& BonesToDisable, bool RefreshHitActors, FMeleeControlsKey Key);
	FOnInputLimitsResetDelegate& OnInputLimitsResetDelegate(){ return AcceptedInputs.OnInputLimitsReset; }

	void GetAvailableAttacks(TArray<FAttackProperties>& AvailableAttacks) const
		{ AvailableAttacks = CharacterStats->AvailableAttacks; }
	bool ExecuteAttack(const FAttackProperties& AttackProperties);
	
	UFUNCTION(BlueprintCallable, Category = Combat)
	void ExecuteAttack(int32 Index);
	
protected:
	bool bIsInvincible;
	TArray<FName> MeleeEnabledBones;
	FCharacterStats* CharacterStats;
	FTimerHandle InvincibilityHandle;

	UPROPERTY()
	TArray<AActor*> RecentlyDamagedActors;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	UTargetInformationComponent* TargetInformationComponent;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	FCharacterBaseStats BaseStats;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* GetHitAnimation;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* DeathAnimation;

	UPROPERTY(EditAnywhere, Category = Combat)
	TSubclassOf<UBoneSoundResponseConfig> BoneSoundResponseConfig;

	UPROPERTY(EditAnywhere, Category = Combat)
	UNiagaraSystem* GetHitFX;
	
	UPROPERTY(EditAnywhere, Category = Combat, meta=(Units="cm"))
	float HitFXRadius;

	virtual void BeginPlay() override;
	
	void SwitchMovementToWalk() const;
	void SwitchMovementToRun() const;

	virtual void MakeInvincible(float InvincibilityTime);
	void EndInvincibility();
	
	void CheckMeshOverlaps();

	virtual void QueueFollowUpLimit(const TArray<FInputLimits>& InputLimits, int32 CurrentLimitIndex = 0);

	UFUNCTION()
	bool OnCheckCanExecuteAttack(const FAttackProperties& Properties);
	UFUNCTION()
	void OnExecuteAttack(const FAttackProperties& Properties);
	UFUNCTION()
	void OnGetHit(const FCustomDamageEvent& DamageEvent);
	UFUNCTION()
	void OnDeath(const FCustomDamageEvent& DamageEvent);
};
