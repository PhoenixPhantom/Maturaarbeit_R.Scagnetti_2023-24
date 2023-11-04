// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/Stats/StatusEffect.h"

UStatusEffect::UStatusEffect(): EffectTarget(nullptr)
{
}

void UStatusEffect::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	EffectTarget = nullptr;
}

void UStatusEffect::OnEffectApplied_Implementation(AGeneralCharacter* Target)
{
	EffectTarget = Target;
}
