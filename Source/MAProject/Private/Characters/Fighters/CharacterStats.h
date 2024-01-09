// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Attacks/AttackProperties.h"
#include "Attacks/Attacks.h"
#include "Utility/Stats/GeneralStats.h"
#include "CharacterStats.generated.h"


class UAttackTree;

USTRUCT()
struct MAPROJECT_API FSavableCharacterModifiers : public FSavableModifiersBase
{
	GENERATED_BODY()
	
	FSavableCharacterModifiers() : Dummy(0){};

	UPROPERTY(SaveGame)
	int32 Dummy;
};

//This is a helper struct used in tandem with
//FCharacterStats to allow base settings for stats to be tuned in-editor
USTRUCT()
struct MAPROJECT_API FCharacterBaseStats : public FGeneralBaseStats
{
	GENERATED_BODY()
	
	FCharacterBaseStats() : BaseWalkSpeed(600), RunSpeedup(100), DashFactor(2.f), BaseToughness(30),
		BaseInterruptionResistance(0), AttackTree(nullptr)
	{
	}

	FCharacterBaseStats(const FCharacterBaseStats& Source) : BaseWalkSpeed(Source.BaseWalkSpeed),
		RunSpeedup(Source.RunSpeedup), DashFactor(Source.RunSpeedup), BaseToughness(Source.BaseToughness),
		BaseInterruptionResistance(Source.BaseInterruptionResistance), AttackTree(Source.AttackTree)
	{
	};

	bool operator==(const FCharacterBaseStats &CharacterBaseStats) const;

	UPROPERTY(EditDefaultsOnly, meta=(Units="m/s"))
	float BaseWalkSpeed;

	UPROPERTY(EditDefaultsOnly, meta=(Units="%"))
	float RunSpeedup;

	UPROPERTY(EditDefaultsOnly, meta=(ForceUnits="x"))
	float DashFactor;

	UPROPERTY(EditDefaultsOnly)
	int32 BaseToughness;
	
	UPROPERTY(EditDefaultsOnly)
	int32 BaseInterruptionResistance;

	
	UPROPERTY(EditDefaultsOnly)
	UAttackTree* AttackTree;
};

USTRUCT(BlueprintType)
struct FCharacterStatsBuffs : public FGeneralObjectStatsBuffs
{
	GENERATED_BODY()
public:
	FCharacterStatsBuffs();
	FCharacterStatsBuffs(const FGeneralObjectStatsBuffs& GeneralObjectStatsBuffs, float WSB, float RSB,
		float IRB, float TB, float FWS, float FRS, float FIR, float FT);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Units="%"))
	float WalkSpeedBuff;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Units="%"))
	float RunSpeedBuff;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Units="%"))
	float InterruptionResBuff;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Units="%"))
	float ToughnessBuff;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Units="m/s"))
	float FlatWalkSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Units="m/s"))
	float FlatRunSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FlatInterruptionRes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FlatToughness;

	FCharacterStatsBuffs ReverseCharacterStatsBuffs() const;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnToughnessChangedDelegate, int32, New, int32, Old);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMaxToughnessChangedDelegate, int32, NewCurrent, int32, NewMax);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMinToughnessReachedDelegate);

struct FCharacterStats : public FGeneralObjectStats
{
	typedef FGeneralObjectStats Super;
	FCharacterStats();

	FOnToughnessChangedDelegate OnToughnessChanged;
	FOnMaxToughnessChangedDelegate OnMaxToughnessChanged;
	FOnMinToughnessReachedDelegate OnNoToughnessReached;
	
	TScalable<float, float> WalkSpeed;
	TScalable<float, float> RunSpeed;
	float RunSpeedupFactor;
	float DashFactor;
	TScalable<int32, float> InterruptionResistance;
	TMaxedValue<int32, float> Toughness;

	FAttacks Attacks;

	void FromBase(const FCharacterBaseStats& Stats, const FSavableCharacterModifiers& Modifiers, UObject* Outer);
	virtual void Reset() override;
	void ResetToughness();
	void ResetRunSpeed();
	void ResetWalkSpeed();
	void ResetInterruptionResistance();
	void RecalculateBaseRunSpeed();
	float GetDashSpeed() const { return RunSpeed.GetResulting() * DashFactor; }
	void Buff(const FCharacterStatsBuffs& Buffs);
	void Debuff(const FCharacterStatsBuffs& Buffs);
	void ReduceToughness(int32 ToughnessBreak);
	void RefillToughness();

	virtual float GetDamageOutput() const override;
	virtual void GenerateDamageEvent(FCustomDamageEvent& DamageEvent, const FHitResult& HitResult = FHitResult()) const override;
	uint32 ReceiveDamage(float Damage, const FAttackDamageEvent* DamageInfo);

	
	bool operator==(const FCharacterStats& CharacterStats) const;
};

