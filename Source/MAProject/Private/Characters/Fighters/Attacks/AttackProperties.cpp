// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackProperties.h"

FAttackProperties::FAttackProperties() : DamagePercent(100.f), CdTime(0.f), MaximalMovementDistance(200.f),
	DefaultMovementDistance(100.f), Priority(1.f), AtkAnimation(nullptr), World(nullptr), bIsOnCd(false)
{
	InputLimits.SetNum(0, true);
	InputLimits.Add({EInputType::Attack, 1.f});
}

FAttackProperties::FAttackProperties(const FAttackProperties& Properties) : DamagePercent(Properties.DamagePercent),
            CdTime(Properties.CdTime), DamageEvent(Properties.DamageEvent),
			MaximalMovementDistance(Properties.MaximalMovementDistance),
			DefaultMovementDistance(Properties.DefaultMovementDistance), Priority(Properties.Priority),
			AtkAnimation(Properties.AtkAnimation), InputLimits(Properties.InputLimits),
			World(Properties.World), bIsOnCd(Properties.bIsOnCd)
{
}

bool FAttackProperties::operator==(const FAttackProperties& AttackProperties) const
{
	return DamagePercent == AttackProperties.DamagePercent && CdTime == AttackProperties.CdTime &&
		DamageEvent == AttackProperties.DamageEvent &&
		MaximalMovementDistance == AttackProperties.MaximalMovementDistance &&
		DefaultMovementDistance == AttackProperties.DefaultMovementDistance &&
		Priority == AttackProperties.Priority && AtkAnimation == AttackProperties.AtkAnimation &&
		InputLimits == AttackProperties.InputLimits;
}

float FAttackProperties::GetPriority(float DistanceFromTarget) const
{
	return Priority / std::max(1.f,
		(DistanceFromTarget-DefaultMovementDistance)/(MaximalMovementDistance-DefaultMovementDistance));
}

void FAttackProperties::Execute()
{
	bIsOnCd = true;
	const float ActualCdTime = GetTotalCdTime();
	check(ActualCdTime > 0.f);
	World->GetTimerManager().SetTimer(CdHandle,
		[this](){ bIsOnCd = false; },ActualCdTime, false);
}

float FAttackProperties::CdTimeElapsed() const
{
	if(bIsOnCd) return 0.f;
	return World->GetTimerManager().GetTimerElapsed(CdHandle);
}

float FAttackProperties::CdTimeRemaining() const
{
	if(bIsOnCd) return 0.f;
	return World->GetTimerManager().GetTimerRemaining(CdHandle);
}

float FAttackProperties::GetTotalCdTime() const
{
	return AtkAnimation->GetPlayLength() + CdTime;
}