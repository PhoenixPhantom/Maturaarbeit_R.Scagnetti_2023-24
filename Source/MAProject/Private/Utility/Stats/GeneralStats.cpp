// Fill out your copyright notice in the Description page of Project Settings.


#include "GeneralStats.h"

#include "Characters/Fighters/Attacks/AttackDamageEvent.h"

bool FGeneralBaseStats::operator==(const FGeneralBaseStats& GeneralBaseStats) const
{
	return BaseHealth == GeneralBaseStats.BaseHealth && BaseAttack == GeneralBaseStats.BaseAttack &&
		BaseDefense == GeneralBaseStats.BaseDefense;
}

FGeneralObjectStats::FGeneralObjectStats(): Attack(0, 0, 0.f),
	Defense(0, 0, 0.f)
{
}

bool FGeneralObjectStats::operator==(const FGeneralObjectStats& GeneralObjectStats) const
{
	return Health == GeneralObjectStats.Health && Attack == GeneralObjectStats.Attack &&
		Defense == GeneralObjectStats.Defense;
}

void FGeneralObjectStats::FromBase(const FGeneralBaseStats& Stats, const FSavableModifiersBase& Modifiers)
{
	//TODO: Include real level scaling
	Health.SetBaseMax(Stats.BaseHealth * Modifiers.Level);
	Attack.Base = Stats.BaseAttack * Modifiers.Level;
	Defense.Base = Stats.BaseDefense * Modifiers.Level;
}

void FGeneralObjectStats::Buff(const FGeneralObjectStatsBuffs& Buffs)
{
	Health.AddBonuses(Buffs.FlatHealth, Buffs.HealthBuff);
	OnMaxHealthChanged.Broadcast(Health.Current, Health.Maximum.GetResulting());
	Attack.FlatBonus += FMath::Floor(Buffs.FlatAttack);
	Defense.FlatBonus += FMath::Floor(Buffs.FlatDefense);
	Attack.PercentageBonus += Buffs.AttackBuff;
	Defense.PercentageBonus += Buffs.DefenseBuff;
}

void FGeneralObjectStats::Debuff(const FGeneralObjectStatsBuffs& Buffs)
{
	Health.AddBonuses(-Buffs.FlatHealth, -Buffs.HealthBuff);
	OnMaxHealthChanged.Broadcast(Health.Current, Health.Maximum.GetResulting());
	Attack.FlatBonus -= FMath::Floor(Buffs.FlatAttack);
	Defense.FlatBonus -= FMath::Floor(Buffs.FlatDefense);
	Attack.PercentageBonus -= Buffs.AttackBuff;
	Defense.PercentageBonus -= Buffs.DefenseBuff;
}

float FGeneralObjectStats::GetDamageOutput() const
{
	return Attack.GetResulting();
}

int32 FGeneralObjectStats::ReceiveDamage(float Damage, const FCustomDamageEvent* DamageInfo)
{	
	if(Damage > 0.f)
	{
#if USE_UE5_DELEGATE
		if(OnGetDamaged.IsBound()) OnGetDamaged.Broadcast(*DamageInfo);
#else
		// ReSharper disable once CppExpressionWithoutSideEffects
		OnGetDamaged.ExecuteIfBound(DamageInfo);
#endif
	}
	return ReceiveDamage(Damage);
}

int32 FGeneralObjectStats::ReceiveTrueDamage(int32 Damage)
{
	OnHealthChanged.Broadcast(Health.Current <= Damage ? 0 : Health.Current - Damage,
		Health.Current);
	
	if(Health.Current <= Damage)
	{
		Health.Current = 0;
		if(OnNoHealthReached.IsBound()) OnNoHealthReached.Broadcast();
		return 0;
	}
	
	Health.Current -= Damage;
	return Health.Current;
}
