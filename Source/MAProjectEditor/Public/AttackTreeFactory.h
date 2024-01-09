// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "AttackTreeFactory.generated.h"

/**
 * 
 */
UCLASS()
class MAPROJECTEDITOR_API UAttackTreeFactory : public UFactory
{
	GENERATED_BODY()
public:
	UAttackTreeFactory();

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags,
		UObject* Context, FFeedbackContext* Warn) override;
	virtual FText GetDisplayName() const override;
	virtual FString GetDefaultNewAssetName() const override;
};
