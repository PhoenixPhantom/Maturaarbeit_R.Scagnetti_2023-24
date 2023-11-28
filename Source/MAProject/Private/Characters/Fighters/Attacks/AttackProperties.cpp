// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackProperties.h"


FAttackProperties::FAttackProperties() : DamagePercent(100.f), CdTime(0.f), MaximalMovementDistance(200.f),
	DefaultMovementDistance(100.f), MaxComboTime(0), Priority(1.f), bIsMeleeAttack(true), AtkAnimation(nullptr)/*,
	World(nullptr), bIsOnCd(false)*/
{
	InputLimits.SetNum(0, true);
	InputLimits.Add({EInputType::Attack, 1.f});
}

FAttackProperties::FAttackProperties(const FAttackProperties& Properties) : DamagePercent(Properties.DamagePercent),
	CdTime(Properties.CdTime), DamageEvent(Properties.DamageEvent),
	MaximalMovementDistance(Properties.MaximalMovementDistance),
	DefaultMovementDistance(Properties.DefaultMovementDistance), MaxComboTime(Properties.DefaultMovementDistance),
	Priority(Properties.Priority), bIsMeleeAttack(Properties.bIsMeleeAttack), AtkAnimation(Properties.AtkAnimation),
	InputLimits(Properties.InputLimits)/*, World(Properties.World), bIsOnCd(Properties.bIsOnCd)*/
{
}

bool FAttackProperties::operator==(const FAttackProperties& AttackProperties) const
{
	return DamagePercent == AttackProperties.DamagePercent && CdTime == AttackProperties.CdTime &&
		DamageEvent == AttackProperties.DamageEvent &&
		MaximalMovementDistance == AttackProperties.MaximalMovementDistance &&
		DefaultMovementDistance == AttackProperties.DefaultMovementDistance &&
		Priority == AttackProperties.Priority && bIsMeleeAttack == AttackProperties.bIsMeleeAttack &&
		AtkAnimation == AttackProperties.AtkAnimation && InputLimits == AttackProperties.InputLimits;
}

float FAttackProperties::GetOverallValue() const
{
	return DamagePercent / 100.f * (static_cast<float>(DamageEvent.StaggerChance)*0.005f + Priority*2.5f) /
		FMath::Sqrt(AtkAnimation->GetPlayLength());
}

float FAttackProperties::GetTotalCdTime() const
{
	return AtkAnimation->GetPlayLength() + CdTime;
}


