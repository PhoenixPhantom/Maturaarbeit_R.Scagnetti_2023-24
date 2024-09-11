// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DamageEvents.h"
#include "CustomDamageEvent.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogDamageSystem, Error, All);

USTRUCT(BlueprintType)
struct MAPROJECT_API FCustomDamageEvent : public FDamageEvent
{
	GENERATED_BODY()

public:

	//I just assume there is no value >= 10 in the built-in DamageEvents since I can only find 4 different DamageEvents
	static const int32 ClassID = 10;

	virtual int32 GetTypeID() const override { return FCustomDamageEvent::ClassID; }
	virtual bool IsOfType(int32 InID) const override { return FCustomDamageEvent::ClassID == InID || Super::IsOfType(InID); };

	bool operator==(const FCustomDamageEvent& CustomDamageEvent) const;
};
