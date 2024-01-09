// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackDamageEvent.h"

FAttackDamageEvent::FAttackDamageEvent(): ToughnessBreak(0.f), StaggerChance(0),
                                          HitFXScaleFactor(1.f)
{
}

FAttackDamageEvent::FAttackDamageEvent(const FAttackDamageEvent& AttackDamageEvent): Super(AttackDamageEvent),
	ToughnessBreak(AttackDamageEvent.ToughnessBreak), StaggerChance(AttackDamageEvent.StaggerChance),
	HitFXScaleFactor(AttackDamageEvent.HitFXScaleFactor), HitDirection(AttackDamageEvent.HitDirection),
	HitLocation(AttackDamageEvent.HitLocation), OnHitRegistered(AttackDamageEvent.OnHitRegistered)
{
}

bool FAttackDamageEvent::operator==(const FAttackDamageEvent& AttackDamageEvent) const
{
	return Super::operator==(AttackDamageEvent) && ToughnessBreak == AttackDamageEvent.ToughnessBreak &&
		StaggerChance == AttackDamageEvent.StaggerChance &&	HitFXScaleFactor == AttackDamageEvent.HitFXScaleFactor &&
		HitDirection == AttackDamageEvent.HitDirection && HitLocation == AttackDamageEvent.HitLocation &&
		OnHitRegistered.GetHandle() == AttackDamageEvent.OnHitRegistered.GetHandle();
}
