// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackProperties.h"


FAttackProperties::FAttackProperties() : DamagePercent(100.f), CdTime(0.f), MaximalMovementDistance(200.f),
    DefaultMovementDistance(100.f), MaxComboTime(0), Priority(1.f), AtkAnimation(nullptr), World(nullptr), bIsOnCd(false)
{
	InputLimits.SetNum(0, true);
	InputLimits.Add({EInputType::Attack, 1.f});
}

FAttackProperties::FAttackProperties(const FAttackProperties& Properties) : DamagePercent(Properties.DamagePercent),
	CdTime(Properties.CdTime), DamageEvent(Properties.DamageEvent),
	MaximalMovementDistance(Properties.MaximalMovementDistance),
	DefaultMovementDistance(Properties.DefaultMovementDistance), MaxComboTime(Properties.DefaultMovementDistance),
	Priority(Properties.Priority), AtkAnimation(Properties.AtkAnimation), InputLimits(Properties.InputLimits),
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
	return DistanceFromTarget <= DefaultMovementDistance ? Priority : Priority *
		(1.f + (DistanceFromTarget-DefaultMovementDistance) / (MaximalMovementDistance-DefaultMovementDistance));
}

float FAttackProperties::GetOverallValue() const
{
	return DamagePercent / 100.f * (static_cast<float>(DamageEvent.StaggerChance)*0.005f + Priority*2.5f) /
		FMath::Sqrt(AtkAnimation->GetPlayLength());
}

void FAttackProperties::Execute(UWorld* WorldContext)
{
	check(IsValid(WorldContext));
	World = WorldContext;
	bIsOnCd = true;
	const float ActualCdTime = GetTotalCdTime();
	check(ActualCdTime > 0.f);
	World->GetTimerManager().SetTimer(CdHandle,
		[this](){ bIsOnCd = false; },ActualCdTime, false);
}

float FAttackProperties::CdTimeElapsed() const
{
	if(!bIsOnCd || !IsValid(World)) return -1.f;
	return World->GetTimerManager().GetTimerElapsed(CdHandle);
}

float FAttackProperties::CdTimeRemaining() const
{
	if(!bIsOnCd || !IsValid(World)) return -1.f;
	return World->GetTimerManager().GetTimerRemaining(CdHandle);
}

float FAttackProperties::GetTotalCdTime() const
{
	return AtkAnimation->GetPlayLength() + CdTime;
}


