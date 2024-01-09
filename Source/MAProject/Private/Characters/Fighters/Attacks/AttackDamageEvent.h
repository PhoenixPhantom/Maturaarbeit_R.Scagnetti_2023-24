// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Utility/Stats/CustomDamageEvent.h"
#include "AttackDamageEvent.generated.h"

USTRUCT(BlueprintType)
struct MAPROJECT_API FAttackDamageEvent : public FCustomDamageEvent
{
	GENERATED_BODY()

public:

	FAttackDamageEvent();
	FAttackDamageEvent(const FAttackDamageEvent& AttackDamageEvent);

	UPROPERTY(EditDefaultsOnly, meta=(UIMin="0", ClampMin="0"))
	int32 ToughnessBreak;
	///The chance of a stagger occuring (in per mille)
	UPROPERTY(EditDefaultsOnly, meta=(UIMax="1000", UIMin="0", ClampMax="1000", ClampMin="0"))
	int32 StaggerChance;
	UPROPERTY(EditDefaultsOnly, meta=(ForceUnits="x"))
	float HitFXScaleFactor;
	
	FVector HitDirection;
	FVector HitLocation;

	TDelegate<void(bool)> OnHitRegistered;
	
	static const int32 ClassID = 11;

	virtual int32 GetTypeID() const override { return FAttackDamageEvent::ClassID; }
	virtual bool IsOfType(int32 InID) const override { return FAttackDamageEvent::ClassID == InID || Super::IsOfType(InID); }

	bool operator==(const FAttackDamageEvent& AttackDamageEvent) const;
};
