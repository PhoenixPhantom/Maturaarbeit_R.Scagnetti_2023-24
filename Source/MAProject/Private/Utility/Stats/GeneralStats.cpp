// Fill out your copyright notice in the Description page of Project Settings.


#include "GeneralStats.h"

#include "Characters/Fighters/Attacks/AttackDamageEvent.h"

bool FGeneralBaseStats::operator==(const FGeneralBaseStats& GeneralBaseStats) const
{
	return BaseHealth == GeneralBaseStats.BaseHealth && BaseAttack == GeneralBaseStats.BaseAttack &&
		BaseDefense == GeneralBaseStats.BaseDefense;
}

FGeneralObjectStats::FGeneralObjectStats(): OnGetDamaged(), Health(0),
	MaxHealth(0, 0, 0.f), Attack(0, 0, 0.f),
	Defense(0, 0, 0.f)
{
}

bool FGeneralObjectStats::operator==(const FGeneralObjectStats& GeneralObjectStats) const
{
	return Health == GeneralObjectStats.Health && MaxHealth == GeneralObjectStats.MaxHealth &&
		Attack == GeneralObjectStats.Attack && Defense == GeneralObjectStats.Defense;
}

void FGeneralObjectStats::FromBase(const FGeneralBaseStats& Stats, const FSavableModifiersBase& Modifiers)
{
	//TODO: Include real level scaling
	MaxHealth.Base = Stats.BaseHealth * Modifiers.Level;
	Health = MaxHealth.GetResulting();
	Attack.Base = Stats.BaseAttack * Modifiers.Level;
	Defense.Base = Stats.BaseDefense * Modifiers.Level;
}

float FGeneralObjectStats::GetDamageOutput() const
{
	return Attack.GetResulting();
}

uint32 FGeneralObjectStats::ReceiveDamage(float Damage, const FCustomDamageEvent* DamageInfo)
{
	const uint32 DeltaHealth = Damage/static_cast<float>(Defense.GetResulting());
	
	OnHealthChanged.Broadcast(Health <= DeltaHealth ? 0 : Health - DeltaHealth, Health);

	
	if(Damage > 0.f)
	{
#if USE_UE5_DELEGATE
		if(OnGetDamaged.IsBound()) OnGetDamaged.Broadcast(*DamageInfo);
#else
		// ReSharper disable once CppExpressionWithoutSideEffects
		OnGetDamaged.ExecuteIfBound(DamageInfo);
#endif
	}
	
	if(Health <= DeltaHealth)
	{
		Health = 0;
		if(OnNoHealthReached.IsBound()) OnNoHealthReached.Broadcast(*DamageInfo);
		return 0;
	}
	
	Health -= DeltaHealth;
	return Health;
}
