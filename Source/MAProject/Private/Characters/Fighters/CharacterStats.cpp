// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/CharacterStats.h"

bool FCharacterBaseStats::operator==(const FCharacterBaseStats& CharacterBaseStats) const
{
	return Super::operator==(CharacterBaseStats) && BaseWalkSpeed == CharacterBaseStats.BaseWalkSpeed &&
		RunSpeedup == CharacterBaseStats.RunSpeedup && DashFactor == CharacterBaseStats.DashFactor &&
		BaseInterruptionResistance == CharacterBaseStats.BaseInterruptionResistance &&
		AttackTree == CharacterBaseStats.AttackTree;
}

FCharacterStats::FCharacterStats() :
	WalkSpeed(0.f, 0.f, 0.f), RunSpeed(0.f, 0.f, 0.f),
	DashFactor(2.f), InterruptionResistance(0.f, 0.f, 0.f)
{
}

void FCharacterStats::FromBase(const FCharacterBaseStats& Stats, const FSavableCharacterModifiers& Modifiers, UObject* Outer)
{
	FGeneralObjectStats::FromBase(Stats, Modifiers);
	WalkSpeed.Base = Stats.BaseWalkSpeed;
	RunSpeed.Base = (1.f + Stats.RunSpeedup/100.f) * Stats.BaseWalkSpeed;
	DashFactor = Stats.DashFactor;

	InterruptionResistance.Base = Stats.BaseInterruptionResistance;
	Attacks = FAttacks(Stats.AttackTree, Outer);
}

float FCharacterStats::GetDamageOutput() const
{
	return FGeneralObjectStats::GetDamageOutput() * Attacks.GetLatestAttackProperties().DamagePercent / 100.f;
}

void FCharacterStats::GenerateDamageEvent(FCustomDamageEvent& DamageEvent, const FHitResult& HitResult) const
{
	check(DamageEvent.IsOfType(FAttackDamageEvent::ClassID));
	FAttackDamageEvent& AttackDamageEvent = static_cast<FAttackDamageEvent&>(DamageEvent);
	AttackDamageEvent = Attacks.GetLatestAttackProperties().DamageEvent;
	AttackDamageEvent.HitDirection = HitResult.Location - HitResult.TraceStart;
	AttackDamageEvent.HitLocation = HitResult.Location;
}

uint32 FCharacterStats::ReceiveDamage(float Damage, const FAttackDamageEvent* DamageInfo)
{
	const uint32 RemainingHealth = FGeneralObjectStats::ReceiveDamage(Damage, DamageInfo);
	return RemainingHealth;
}


bool FCharacterStats::operator==(const FCharacterStats& CharacterStats) const
{
	return FGeneralObjectStats::operator==(CharacterStats) && WalkSpeed == CharacterStats.WalkSpeed &&
		RunSpeed == CharacterStats.RunSpeed && DashFactor == CharacterStats.DashFactor &&
		InterruptionResistance == CharacterStats.InterruptionResistance && Attacks == CharacterStats.Attacks;
}