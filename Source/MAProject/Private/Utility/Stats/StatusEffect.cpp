// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/Stats/StatusEffect.h"

UStatusEffect::UStatusEffect() : EffectTarget(nullptr)
{
}

void UStatusEffect::OnEffectRemoved_Implementation(AGeneralCharacter* Target)
{
}

void UStatusEffect::OnEffectApplied_Implementation(AGeneralCharacter* Target)
{
	EffectTarget = Target;
}
