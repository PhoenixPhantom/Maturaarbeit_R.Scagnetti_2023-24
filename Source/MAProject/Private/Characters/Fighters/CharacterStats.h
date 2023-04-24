// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Attacks/AttackProperties.h"
#include "Utility/Stats/GeneralStats.h"
#include "CharacterStats.generated.h"


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
	
	FCharacterBaseStats() : BaseWalkSpeed(600), DashSpeedup(100), BaseInterruptionResistance(0){}

	FCharacterBaseStats(const FCharacterBaseStats& Source) : BaseWalkSpeed(Source.BaseWalkSpeed),
	DashSpeedup(Source.DashSpeedup), BaseInterruptionResistance(Source.BaseInterruptionResistance){};

	bool operator==(const FCharacterBaseStats &CharacterBaseStats) const;

	UPROPERTY(EditAnywhere, meta=(Units="m/s"))
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere, meta=(Units="%"))
	float DashSpeedup;

	
	UPROPERTY(EditAnywhere)
	uint32 BaseInterruptionResistance;

	UPROPERTY(EditAnywhere)
	TArray<FAttackProperties> AvailableAttacks;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExecuteAttackDelegate, FAttackProperties&, AttackProperties);

struct FCharacterStats : public FGeneralObjectStats
{
	FCharacterStats();

	FOnExecuteAttackDelegate OnExecuteAttack;
	
	TScalable<TScalable<float, float>, float> MovementSpeed;
	TScalable<uint32, float> InterruptionResistance;

	TArray<FAttackProperties> AvailableAttacks;

	void FromBase(const FCharacterBaseStats& Stats, const FSavableCharacterModifiers& Modifiers, UWorld* World);
	void ExecuteAttack(int32 Index);

	virtual float GetDamageOutput() const override;
	virtual FCustomDamageEvent* GenerateDamageEvent(const FHitResult& HitResult = FHitResult()) const override;
	uint32 ReceiveDamage(float Damage, const FAttackDamageEvent& DamageInfo);

	
	bool operator==(const FCharacterStats& CharacterStats) const;
protected:
	FAttackProperties* CurrentAttack;
};

