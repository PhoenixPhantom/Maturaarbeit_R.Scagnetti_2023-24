// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomDamageEvent.h"

DEFINE_LOG_CATEGORY(LogDamageSystem);

bool FCustomDamageEvent::operator==(const FCustomDamageEvent& CustomDamageEvent) const
{
	return DamageTypeClass == CustomDamageEvent.DamageTypeClass;
}
