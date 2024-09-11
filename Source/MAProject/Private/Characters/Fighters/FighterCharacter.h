
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterStats.h"
#include "GenericTeamAgentInterface.h"
#include "Characters/GeneralCharacter.h"
#include "FighterCharacter.generated.h"

class UAttackTree;
class UStatsMonitorBaseWidget;
class UBoneSoundResponseConfig;
class UTargetInformationComponent;
class UNiagaraSystem;

struct FSetWalkOrRunKey final
{
	friend class UBTTask_CustomMoveTo;
	friend class AFighterCharacter;
	friend class APlayerCharacter;
private:
	FSetWalkOrRunKey(){}
};

struct FMeleeControlsKey final
{
	friend class UAnimNotifyState_MeleeAttack;
private:
	FMeleeControlsKey(){}
};

struct FModifyInputLimitsKey final
{
	friend class UBTTask_ExecuteAttackTask;
private:
	FModifyInputLimitsKey(){}
};

/**
 * 
 */
UCLASS()
class AFighterCharacter : public AGeneralCharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	AFighterCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float DeltaSeconds) override;
	
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
		AActor* DamageCauser) override;

	virtual void StopAnimMontage(UAnimMontage* AnimMontage = nullptr) override;

	bool IsMovingOnFloor() const;
	bool IsWalking() const;
	bool IsRunning() const;

	/// Blend the time dilation from default (1.f) to resulting and back
	void BlendTimeDilation(float BlendTime, float TotalTime, float TargetDilation);
	
	void ActivateMeleeBones(const TArray<FName>& BonesToEnable, bool StartEmpty, bool AllowHitRecentVictims,
		FMeleeControlsKey Key);
	void DeactivateMeleeBones(const TArray<FName>& BonesToDisable, bool IsLastAttackOfAnimation, FMeleeControlsKey Key);
	void AddOnInputLimitsResetDelegate(const TDelegate<void(bool)>& FunctionToAdd, FModifyInputLimitsKey);
	void RemoveOnInputLimitsResetDelegate(const TDelegate<void(bool)>& FunctionToAdd, FModifyInputLimitsKey);

	const FCharacterStats* GetCharacterStats() const { return CharacterStats; }
	
	void SwitchMovementToWalk(FSetWalkOrRunKey) const;
	void SwitchMovementToRun(FSetWalkOrRunKey) const;
	
protected:
	uint8 bIsInvincible:1;
	float SourceTimeDilation;
	float TargetTimeDilation;
	float TimeDilationBlendTime;
	float TimeDilationTotalTime;
	float TimeDilationEffectTimeRemaining;

	
	TArray<FName> MeleeEnabledBones;
	FCharacterStats* CharacterStats;
	FTimerHandle InvincibilityHandle;
	float ToughnessBrokenTime;

	UPROPERTY()
	TArray<AActor*> RecentlyDamagedActors;

	UPROPERTY()
	UStatsMonitorBaseWidget* StatsMonitorWidget;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	UTargetInformationComponent* TargetInformationComponent;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	FCharacterBaseStats BaseStats;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* GetHitAnimation;

	UPROPERTY(EditAnywhere, Category = Combat)
	TSubclassOf<UBoneSoundResponseConfig> BoneSoundResponseConfig;

	UPROPERTY(EditAnywhere, Category = Combat)
	UNiagaraSystem* GetHitFX;
	
	UPROPERTY(EditAnywhere, Category = Combat, meta=(Units="cm"))
	float HitFXRadius;

	virtual void BeginPlay() override;

	virtual void MakeInvincible(float InvincibilityTime);
	void EndInvincibility();
	
	void CheckMeshOverlaps();
	void ProcessTimeDilation(float DeltaSeconds);

	virtual void GenerateDamageEvent(FAttackDamageEvent& AttackDamageEvent, const FHitResult& CausingHit);
	virtual bool TriggerDeath() override;
	virtual void PlayHitSound(const FVector& HitLocation);
	virtual void SpawnHitFX(const FVector& Location, float ScaleFactor);
	virtual void GetStaggered(bool HeavyStagger);
	virtual void OnGetDamaged(const FCustomDamageEvent* DamageEvent);
	virtual bool TriggerToughnessBroken();
	virtual void RestoreToughness();
	virtual void OnGetAttacked(const FAttackDamageEvent* DamageEvent);
	virtual void OnAttackTreeModeChanged(FString NewRoot){}

	virtual void QueueFollowUpLimit(const TArray<FNewInputLimits>& InputLimits);

	virtual void OnHitTimeDilation(bool WasStaggered);
	
	UFUNCTION()
	void RegisterHealthInfoWidget(UStatsMonitorBaseWidget* Widget);

	UFUNCTION(BlueprintCallable)
	void SetAttackTreeMode(FString ModeIdentifier);
	
	UFUNCTION(BlueprintPure)
	float GetMaxHealthBlueprint() const { return CharacterStats->Health.Maximum.GetResulting(); }

	UFUNCTION(BlueprintCallable)
	void ApplyBuffTimed(const FCharacterStatsBuffs& Buffs, float Duration = 1.f);
	UFUNCTION(BlueprintCallable)
	void ApplyBuff(const FCharacterStatsBuffs& Buffs);
	UFUNCTION(BlueprintCallable)
	void RemoveBuff(bool bResetHealth, bool bResetAttack, bool bResetDefense, bool bResetToughness, bool bResetRunSpeed,
		bool bResetWalkSpeed, bool bResetInterruptionResistance);
	UFUNCTION(BlueprintCallable)
	void ForceSetCd(FString NodeName, float CdTime, bool ChangeBy = false);
	UFUNCTION(BlueprintNativeEvent)
	void OnHealthChanged(int32 NewHealth, int32 OldHealth);
	UFUNCTION()
	bool OnCheckCanExecuteAttack(const FAttackProperties& Properties);
	UFUNCTION()
	void OnExecuteAttack(const FAttackProperties& Properties);
	UFUNCTION()
	void OnGetDamagedUE(const FCustomDamageEvent& DamageEvent){ OnGetDamaged(&DamageEvent);}
	UFUNCTION()
	void OnToughnessBroken(){ TriggerToughnessBroken(); };
	UFUNCTION()
	void OnDeath(){ TriggerDeath(); };
};
