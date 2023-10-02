// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackDamageEvent.h"

FAttackDamageEvent::FAttackDamageEvent(): StaggerChance(0), StaggerImpulse(10.f), HitFXScaleFactor(1.f)
{}

FAttackDamageEvent::FAttackDamageEvent(const FAttackDamageEvent& AttackDamageEvent): Super(AttackDamageEvent),
	StaggerChance(AttackDamageEvent.StaggerChance), StaggerImpulse(AttackDamageEvent.StaggerImpulse),
	HitFXScaleFactor(AttackDamageEvent.HitFXScaleFactor), HitDirection(AttackDamageEvent.HitDirection),
	HitLocation(AttackDamageEvent.HitLocation)
{}

bool FAttackDamageEvent::operator==(const FAttackDamageEvent& AttackDamageEvent) const
{
	return Super::operator==(AttackDamageEvent) && StaggerChance == AttackDamageEvent.StaggerChance &&
		StaggerImpulse == AttackDamageEvent.StaggerImpulse && HitFXScaleFactor == AttackDamageEvent.HitFXScaleFactor &&
		HitDirection == AttackDamageEvent.HitDirection && HitLocation == AttackDamageEvent.HitLocation;
}
