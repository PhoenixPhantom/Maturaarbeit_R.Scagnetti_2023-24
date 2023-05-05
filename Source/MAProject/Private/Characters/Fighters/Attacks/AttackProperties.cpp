// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackProperties.h"

FAttackProperties::FAttackProperties() : DamagePercent(100.f), CdTime(0.f), MaximalMotionWarpingDistance(200.f),
    AtkAnimation(nullptr), ResultingLimits(EInputType::Attack), World(nullptr), bIsOnCd(false)
{
}

FAttackProperties::FAttackProperties(const FAttackProperties& Properties): DamagePercent(Properties.DamagePercent),
    CdTime(Properties.CdTime), DamageEvent(Properties.DamageEvent),
	MaximalMotionWarpingDistance(Properties.MaximalMotionWarpingDistance), AtkAnimation(Properties.AtkAnimation),
	ResultingLimits(Properties.ResultingLimits), World(Properties.World), bIsOnCd(Properties.bIsOnCd)
{
}

bool FAttackProperties::operator==(const FAttackProperties& AttackProperties) const
{
	return DamagePercent == AttackProperties.DamagePercent && CdTime == AttackProperties.CdTime &&
		DamageEvent == AttackProperties.DamageEvent &&
		MaximalMotionWarpingDistance == AttackProperties.MaximalMotionWarpingDistance &&
		AtkAnimation == AttackProperties.AtkAnimation && ResultingLimits == AttackProperties.ResultingLimits;
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