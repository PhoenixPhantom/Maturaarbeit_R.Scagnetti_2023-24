// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackProperties.h"


FAttackProperties::FAttackProperties() : DamagePercent(100.f), CdTime(0.f), MaximalMovementDistance(200.f),
	DefaultMovementDistance(100.f), MinimalMovementDistance(0.f), MaxComboTime(0), Priority(1.f), bIsMeleeAttack(true),
	AtkAnimation(nullptr)
{
	InputLimits.SetNum(0, true);
	InputLimits.Add({EInputType::Attack, 1.f});
}

FAttackProperties::FAttackProperties(const FAttackProperties& Properties) : DamagePercent(Properties.DamagePercent),
	CdTime(Properties.CdTime), DamageEvent(Properties.DamageEvent),
	MaximalMovementDistance(Properties.MaximalMovementDistance),
	DefaultMovementDistance(Properties.DefaultMovementDistance), MinimalMovementDistance(Properties.MinimalMovementDistance),
	MaxComboTime(Properties.DefaultMovementDistance), Priority(Properties.Priority), bIsMeleeAttack(Properties.bIsMeleeAttack),
	AtkAnimation(Properties.AtkAnimation), InputLimits(Properties.InputLimits)
{
}

bool FAttackProperties::operator==(const FAttackProperties& AttackProperties) const
{
	return DamagePercent == AttackProperties.DamagePercent && CdTime == AttackProperties.CdTime &&
		DamageEvent == AttackProperties.DamageEvent &&
		MaximalMovementDistance == AttackProperties.MaximalMovementDistance &&
		DefaultMovementDistance == AttackProperties.DefaultMovementDistance &&
		MinimalMovementDistance == AttackProperties.MinimalMovementDistance &&
		Priority == AttackProperties.Priority && bIsMeleeAttack == AttackProperties.bIsMeleeAttack &&
		AtkAnimation == AttackProperties.AtkAnimation && InputLimits == AttackProperties.InputLimits;
}

float FAttackProperties::GetOverallValue() const
{
	float RequiredTimeToExecute = 0.f;
	for(const FNewInputLimits& InputLimit : InputLimits)
	{
		RequiredTimeToExecute += InputLimit.LimitationDuration;
	}
	if(RequiredTimeToExecute < 0.f) return 0.f;
	
	return (DamagePercent * 0.01f + static_cast<float>(DamageEvent.StaggerChance)*0.005f) /
		FMath::Sqrt(RequiredTimeToExecute);
}

float FAttackProperties::GetTotalCdTime() const
{
	return AtkAnimation->GetPlayLength() + CdTime;
}


