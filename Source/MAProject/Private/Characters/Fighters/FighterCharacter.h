
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterStats.h"
#include "GenericTeamAgentInterface.h"
#include "Characters/GeneralCharacter.h"
#include "FighterCharacter.generated.h"

class UAttackTree;
class UHealthMonitorBaseWidget;
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
	void DeactivateMeleeBones(const TArray<FName>& BonesToDisable, bool RefreshHitActors, FMeleeControlsKey Key);
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
	
	TTuple<double, TArray<AttackIndex>> AttackInputString;

	UPROPERTY()
	TArray<AActor*> RecentlyDamagedActors;

	UPROPERTY()
	UHealthMonitorBaseWidget* HealthInfoWidget;
	
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

	virtual void MakeInvincible(float InvincibilityTime);
	void EndInvincibility();
	
	void CheckMeshOverlaps();
	void ProcessTimeDilation(float DeltaSeconds);

	virtual void GenerateDamageEvent(FAttackDamageEvent& AttackDamageEvent, const FHitResult& CausingHit);
	virtual void OnDeathTriggered();
	virtual void PlayHitSound(const FVector& HitLocation);
	virtual void SpawnHitFX(const FVector& Location, float ScaleFactor);
	virtual void GetStaggered(const FAttackDamageEvent* DamageEvent);

	virtual void QueueFollowUpLimit(const TArray<FInputLimits>& InputLimits);

	virtual void OnHitTimeDilation(bool WasStaggered);
	
	UFUNCTION()
	void RegisterHealthInfoWidget(UHealthMonitorBaseWidget* Widget);

	UFUNCTION()
	bool OnCheckCanExecuteAttack(const FAttackProperties& Properties);
	UFUNCTION()
	void OnExecuteAttack(const FAttackProperties& Properties);
	UFUNCTION()
	void OnGetDamagedUE(const FCustomDamageEvent& DamageEvent){ OnGetDamaged(&DamageEvent);};
	void OnGetDamaged(const FCustomDamageEvent* DamageEvent);
	UFUNCTION()
	void OnDeath(const FCustomDamageEvent& DamageEvent);
};
