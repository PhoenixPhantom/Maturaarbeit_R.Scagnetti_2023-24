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

FCharacterStatsBuffs::FCharacterStatsBuffs(): WalkSpeedBuff(0), RunSpeedBuff(0), InterruptionResBuff(0), ToughnessBuff(0),
                                              FlatWalkSpeed(0), FlatRunSpeed(0), FlatInterruptionRes(0), FlatToughness(0)
{
}

FCharacterStatsBuffs::FCharacterStatsBuffs(const FGeneralObjectStatsBuffs& GeneralObjectStatsBuffs, float WSB,
float RSB, float IRB, float TB, float FWS, float FRS, float FIR, float FT) :
	FGeneralObjectStatsBuffs(GeneralObjectStatsBuffs), WalkSpeedBuff(WSB), RunSpeedBuff(RSB), InterruptionResBuff(IRB),
	ToughnessBuff(TB), FlatWalkSpeed(FWS), FlatRunSpeed(FRS), FlatInterruptionRes(FIR), FlatToughness(FT)
{
}

FCharacterStatsBuffs FCharacterStatsBuffs::ReverseCharacterStatsBuffs() const
{
	return FCharacterStatsBuffs(ReverseGeneralObjectBuffs(), -WalkSpeedBuff, -RunSpeedBuff,
		-InterruptionResBuff, -ToughnessBuff,-FlatWalkSpeed, -FlatRunSpeed,
		-FlatInterruptionRes, -FlatToughness);
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

void FCharacterStats::Reset()
{
	Super::Reset();
	ResetToughness();
	ResetRunSpeed();
	ResetWalkSpeed();
	ResetInterruptionResistance();
}

void FCharacterStats::ResetToughness()
{
	Toughness.Reset();
	OnMaxToughnessChanged.Broadcast(Health.Current, Health.Maximum.GetResulting());
}

void FCharacterStats::ResetRunSpeed()
{
	RunSpeed.FlatBonus = 0.f;
	RunSpeed.PercentageBonus = 0.f;
}

void FCharacterStats::ResetWalkSpeed()
{
	WalkSpeed.FlatBonus = 0.f;
	WalkSpeed.PercentageBonus = 0.f;
	RecalculateBaseRunSpeed();
}

void FCharacterStats::ResetInterruptionResistance()
{
	InterruptionResistance.Base = 0.f;
	InterruptionResistance.PercentageBonus = 0.f;
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
	if(!Attacks.HasPendingAttackProperties())
	{
		checkNoEntry();
		DamageEvent = static_cast<FCustomDamageEvent>(FAttackDamageEvent());
		return;
	}
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