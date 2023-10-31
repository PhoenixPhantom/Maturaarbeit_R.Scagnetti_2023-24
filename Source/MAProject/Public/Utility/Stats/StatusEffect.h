// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "StatusEffect.generated.h"

class AGeneralCharacter;
/** Base class of all status effects
 * Defines how status effects are handled in the most
 * general sense
 */
UCLASS(Blueprintable, BlueprintType)
class MAPROJECT_API UStatusEffect : public UActorComponent
{
	GENERATED_BODY()
public:
	UStatusEffect();

	UFUNCTION()
	virtual void ApplyStatusEffect(AGeneralCharacter* Target) const;
	
	UFUNCTION()
	virtual void RemoveStatusEffect(AGeneralCharacter* Target) const;

protected:
	UPROPERTY(BlueprintReadWrite)
	AGeneralCharacter* EffectTarget;
};
