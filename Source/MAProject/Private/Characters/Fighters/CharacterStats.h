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
	
	FCharacterBaseStats() : BaseWalkSpeed(600), RunSpeedup(100), DashFactor(2.f), BaseInterruptionResistance(0),
	                        AttackTree(nullptr)
	{
	}

	FCharacterBaseStats(const FCharacterBaseStats& Source) : BaseWalkSpeed(Source.BaseWalkSpeed),
	        RunSpeedup(Source.RunSpeedup), DashFactor(Source.RunSpeedup),
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
	uint32 BaseInterruptionResistance;

	
	UPROPERTY(EditDefaultsOnly)
	UAttackTree* AttackTree;
};

struct FCharacterStats : public FGeneralObjectStats
{
	FCharacterStats();
	
	TScalable<float, float> WalkSpeed;
	TScalable<float, float> RunSpeed;
	float DashFactor;
	TScalable<uint32, float> InterruptionResistance;

	FAttacks Attacks;

	void FromBase(const FCharacterBaseStats& Stats, const FSavableCharacterModifiers& Modifiers, UObject* Outer);
	float GetDashSpeed() const { return RunSpeed.GetResulting() * DashFactor; }

	virtual float GetDamageOutput() const override;
	virtual void GenerateDamageEvent(FCustomDamageEvent& DamageEvent, const FHitResult& HitResult = FHitResult()) const override;
	uint32 ReceiveDamage(float Damage, const FAttackDamageEvent* DamageInfo);

	
	bool operator==(const FCharacterStats& CharacterStats) const;
};

