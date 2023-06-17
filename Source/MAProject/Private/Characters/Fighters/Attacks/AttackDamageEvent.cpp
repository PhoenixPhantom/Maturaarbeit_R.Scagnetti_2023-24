// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackDamageEvent.h"

FAttackDamageEvent::FAttackDamageEvent(): StaggerChance(1), StaggerImpulse(10.f)
{}

FAttackDamageEvent::FAttackDamageEvent(const FAttackDamageEvent& AttackDamageEvent): Super(AttackDamageEvent),
	StaggerChance(AttackDamageEvent.StaggerChance), StaggerImpulse(AttackDamageEvent.StaggerImpulse),
	HitDirection(AttackDamageEvent.HitDirection)
{}

bool FAttackDamageEvent::operator==(const FAttackDamageEvent& AttackDamageEvent) const
{
	return Super::operator==(AttackDamageEvent) && StaggerChance == AttackDamageEvent.StaggerChance &&
		StaggerImpulse == AttackDamageEvent.StaggerImpulse && HitDirection == AttackDamageEvent.HitDirection;
}
