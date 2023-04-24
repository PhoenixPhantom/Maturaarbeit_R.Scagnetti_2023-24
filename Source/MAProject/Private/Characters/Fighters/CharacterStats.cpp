// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/CharacterStats.h"

bool FCharacterBaseStats::operator==(const FCharacterBaseStats& CharacterBaseStats) const
{
	return Super::operator==(CharacterBaseStats) && BaseWalkSpeed == CharacterBaseStats.BaseWalkSpeed &&
		DashSpeedup == CharacterBaseStats.DashSpeedup &&
		BaseInterruptionResistance == CharacterBaseStats.BaseInterruptionResistance;
}

FCharacterStats::FCharacterStats() :
	MovementSpeed(TScalable(0.f, 0.f, 0.f),
TScalable(0.f, 0.f, 0.f), 0),
	InterruptionResistance(0.f, 0.f, 0.f), CurrentAttack(nullptr)
{
}

void FCharacterStats::FromBase(const FCharacterBaseStats& Stats, const FSavableCharacterModifiers& Modifiers,
                               UWorld* World)
{
	FGeneralObjectStats::FromBase(Stats, Modifiers);
	MovementSpeed.Base.Base = Stats.BaseWalkSpeed;
	MovementSpeed.PercentageBonus = Stats.DashSpeedup;

	InterruptionResistance.Base = Stats.BaseInterruptionResistance;
	AvailableAttacks = Stats.AvailableAttacks;
	//Add a reference to the world object to every attack (needed to call timers)
	for(FAttackProperties& AttackProperties : AvailableAttacks) AttackProperties.World = World;
}

void FCharacterStats::ExecuteAttack(int32 Index)
{
	if(!AvailableAttacks.IsValidIndex(Index) || AvailableAttacks[Index].GetIsOnCd()) return;
	CurrentAttack = &AvailableAttacks[Index];
	AvailableAttacks[Index].Execute();
	OnExecuteAttack.Broadcast(AvailableAttacks[Index]);
}

float FCharacterStats::GetDamageOutput() const
{
	return FGeneralObjectStats::GetDamageOutput() * CurrentAttack->DamagePercent / 100.f;
}

FCustomDamageEvent* FCharacterStats::GenerateDamageEvent(const FHitResult& HitResult) const
{
	CurrentAttack->DamageEvent.HitDirection = HitResult.Location - HitResult.TraceStart;
	return &CurrentAttack->DamageEvent;
}

uint32 FCharacterStats::ReceiveDamage(float Damage, const FAttackDamageEvent& DamageInfo)
{
	const uint32 RemainingHealth = FGeneralObjectStats::ReceiveDamage(Damage, DamageInfo);
	return RemainingHealth;
}


bool FCharacterStats::operator==(const FCharacterStats& CharacterStats) const
{
	return FGeneralObjectStats::operator==(CharacterStats) && MovementSpeed == CharacterStats.MovementSpeed &&
		InterruptionResistance == CharacterStats.InterruptionResistance;
}