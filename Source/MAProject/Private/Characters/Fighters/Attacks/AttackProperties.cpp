// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackProperties.h"

FAttackProperties::FAttackProperties(): DamagePercent(100.f), CdTime(0.f), AtkAnimation(nullptr), ExhaustionTime(0.f),
	World(nullptr), bIsOnCd(false)
{
}

FAttackProperties::FAttackProperties(const FAttackProperties& Properties):
	DamagePercent(Properties.DamagePercent),
	CdTime(Properties.CdTime), DamageEvent(Properties.DamageEvent), AtkAnimation(Properties.AtkAnimation),
	ExhaustionTime(Properties.ExhaustionTime), World(Properties.World), bIsOnCd(Properties.bIsOnCd)
{
}

bool FAttackProperties::operator==(const FAttackProperties& AttackProperties) const
{
	return DamagePercent == AttackProperties.DamagePercent && CdTime == AttackProperties.CdTime &&
		DamageEvent == AttackProperties.DamageEvent && AtkAnimation == AttackProperties.AtkAnimation &&
		ExhaustionTime == AttackProperties.ExhaustionTime;
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

float FAttackProperties::GetTotalExhaustionTime() const
{
	return AtkAnimation->GetPlayLength() + ExhaustionTime;
}