// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/CharacterStats.h"

bool FCharacterBaseStats::operator==(const FCharacterBaseStats& CharacterBaseStats) const
{
	return Super::operator==(CharacterBaseStats) && BaseWalkSpeed == CharacterBaseStats.BaseWalkSpeed &&
		RunSpeedup == CharacterBaseStats.RunSpeedup && DashFactor == CharacterBaseStats.DashFactor &&
		BaseToughness == CharacterBaseStats.BaseToughness &&
		BaseInterruptionResistance == CharacterBaseStats.BaseInterruptionResistance &&
		AttackTree == CharacterBaseStats.AttackTree;
}

FCharacterStats::FCharacterStats() :
	WalkSpeed(0.f, 0.f, 0.f), RunSpeed(0.f, 0.f, 0.f),
	RunSpeedupFactor(1.f), DashFactor(2.f), InterruptionResistance(0.f, 0.f, 0.f)
{
}

void FCharacterStats::FromBase(const FCharacterBaseStats& Stats, const FSavableCharacterModifiers& Modifiers, UObject* Outer)
{
	Super::FromBase(Stats, Modifiers);
	WalkSpeed.Base = Stats.BaseWalkSpeed;
	RunSpeedupFactor = Stats.RunSpeedup/100.f;
	RecalculateBaseRunSpeed();
	DashFactor = Stats.DashFactor;
	Toughness.SetBaseMax(Stats.BaseToughness);

	InterruptionResistance.Base = Stats.BaseInterruptionResistance;
	Attacks = FAttacks(Stats.AttackTree, Outer);
}

void FCharacterStats::RecalculateBaseRunSpeed()
{
	RunSpeed.Base = (1.f + RunSpeedupFactor) * WalkSpeed.GetResulting();
}

void FCharacterStats::Buff(const FCharacterStatsBuffs& Buffs)
{
	Super::Buff(Buffs);
	WalkSpeed.FlatBonus += Buffs.FlatWalkSpeed;
	WalkSpeed.PercentageBonus += Buffs.WalkSpeedBuff;
	RecalculateBaseRunSpeed();
	RunSpeed.FlatBonus += Buffs.FlatRunSpeed;
	RunSpeed.PercentageBonus += Buffs.RunSpeedBuff;
	InterruptionResistance.FlatBonus += Buffs.FlatInterruptionRes;
	InterruptionResistance.PercentageBonus += Buffs.InterruptionResBuff;
	Toughness.AddBonuses(Buffs.FlatToughness, Buffs.ToughnessBuff);
	OnMaxToughnessChanged.Broadcast(Toughness.Current, Toughness.Maximum.GetResulting());
}

void FCharacterStats::Debuff(const FCharacterStatsBuffs& Buffs)
{
	Super::Debuff(Buffs);
	WalkSpeed.FlatBonus -= Buffs.FlatWalkSpeed;
	WalkSpeed.PercentageBonus -= Buffs.WalkSpeedBuff;
	RecalculateBaseRunSpeed();
	RunSpeed.FlatBonus -= Buffs.FlatRunSpeed;
	RunSpeed.PercentageBonus -= Buffs.RunSpeedBuff;
	InterruptionResistance.FlatBonus -= Buffs.FlatInterruptionRes;
	InterruptionResistance.PercentageBonus -= Buffs.InterruptionResBuff;
	Toughness.AddBonuses(-Buffs.FlatToughness, -Buffs.ToughnessBuff);
	OnMaxToughnessChanged.Broadcast(Toughness.Current, Toughness.Maximum.GetResulting());
}

void FCharacterStats::ReduceToughness(int32 ToughnessBreak)
{
	OnToughnessChanged.Broadcast(Toughness.Current <=  ToughnessBreak ? 0 :
		Toughness.Current - ToughnessBreak, Toughness.Current);
	
	if(Toughness.Current <= ToughnessBreak)
	{
		Toughness.Current = 0;
		if(OnNoToughnessReached.IsBound()) OnNoToughnessReached.Broadcast();
	}
	
	Toughness.Current -= ToughnessBreak;
}

void FCharacterStats::RefillToughness()
{
	OnToughnessChanged.Broadcast(Toughness.Maximum.GetResulting(), Toughness.Current);
	Toughness.Current = Toughness.Maximum.GetResulting();
}

float FCharacterStats::GetDamageOutput() const
{
	check(Attacks.HasPendingAttackProperties());
	return Attacks.HasPendingAttackProperties() ?
		(Super::GetDamageOutput() * Attacks.GetPendingAttackProperties()->DamagePercent / 100.f) : 0.0;
}

void FCharacterStats::GenerateDamageEvent(FCustomDamageEvent& DamageEvent, const FHitResult& HitResult) const
{
	check(DamageEvent.IsOfType(FAttackDamageEvent::ClassID));
	check(Attacks.HasPendingAttackProperties());
	FAttackDamageEvent& AttackDamageEvent = static_cast<FAttackDamageEvent&>(DamageEvent);
	AttackDamageEvent = Attacks.GetPendingAttackProperties()->DamageEvent;
	AttackDamageEvent.HitDirection = HitResult.Location - HitResult.TraceStart;
	AttackDamageEvent.HitLocation = HitResult.Location;
}

uint32 FCharacterStats::ReceiveDamage(float Damage, const FAttackDamageEvent* DamageInfo)
{
	const uint32 RemainingHealth = Super::ReceiveDamage(Damage, DamageInfo);
	ReduceToughness(DamageInfo->ToughnessBreak);
	return RemainingHealth;
}


bool FCharacterStats::operator==(const FCharacterStats& CharacterStats) const
{
	return Super::operator==(CharacterStats) && WalkSpeed == CharacterStats.WalkSpeed &&
		RunSpeed == CharacterStats.RunSpeed && RunSpeedupFactor == CharacterStats.RunSpeedupFactor &&
		DashFactor == CharacterStats.DashFactor && InterruptionResistance == CharacterStats.InterruptionResistance &&
		Toughness == CharacterStats.Toughness && Attacks == CharacterStats.Attacks;
}