// Fill out your copyright notice in the Description page of Project Settings.


#include "GeneralStats.h"

#include "Characters/Fighters/Attacks/AttackDamageEvent.h"

bool FGeneralBaseStats::operator==(const FGeneralBaseStats& GeneralBaseStats) const
{
	return BaseHealth == GeneralBaseStats.BaseHealth && BaseAttack == GeneralBaseStats.BaseAttack &&
		BaseDefense == GeneralBaseStats.BaseDefense;
}

FGeneralObjectStatsBuffs::FGeneralObjectStatsBuffs(float HB, float AB, float DB, float FH, float FA, float FD):
	HealthBuff(HB), AttackBuff(AB), DefenseBuff(DB), FlatHealth(FH), FlatAttack(FA), FlatDefense(FD)
{
}

FGeneralObjectStatsBuffs FGeneralObjectStatsBuffs::ReverseGeneralObjectBuffs() const
{
	return FGeneralObjectStatsBuffs(-HealthBuff, -AttackBuff, -DefenseBuff, -FlatHealth, -FlatAttack,
		-FlatDefense);
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

void FGeneralObjectStats::Reset()
{
	ResetHealth();
	ResetAttack();
	ResetDefense();
}

void FGeneralObjectStats::ResetHealth()
{
	Health.Reset();
	OnMaxHealthChanged.Broadcast(Health.Current, Health.Maximum.GetResulting());
	Health.Reset();
}

void FGeneralObjectStats::ResetAttack()
{
	Attack.FlatBonus = 0.f;
	Attack.PercentageBonus = 0.f;
}

void FGeneralObjectStats::ResetDefense()
{
	Defense.FlatBonus = 0.f;
	Defense.PercentageBonus = 0.f;
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
		// ReSharper disable once CppExpressionWithoutSideEffects
		OnGetDamaged.ExecuteIfBound(DamageInfo);
	}
	return ReceiveDamage(Damage);
}

int32 FGeneralObjectStats::ChangeHealth(int32 DeltaHealth)
{
	int32 ResultingHealth;
	if(Health.Current + DeltaHealth >= Health.Maximum.GetResulting()) ResultingHealth = Health.Maximum.GetResulting();
	else if(Health.Current <= -DeltaHealth) ResultingHealth = 0;
	else ResultingHealth = Health.Current + DeltaHealth;
	
	OnHealthChanged.Broadcast(ResultingHealth, Health.Current);
	
	if(ResultingHealth <= 0 && OnNoHealthReached.IsBound())OnNoHealthReached.Broadcast();
	
	Health.Current = ResultingHealth;
	return Health.Current;
}
