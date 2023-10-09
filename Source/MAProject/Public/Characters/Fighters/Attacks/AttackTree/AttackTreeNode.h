// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericGraphNode.h"
#include "Characters/Fighters/Attacks/AttackProperties.h"
#include "AttackTreeNode.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class MAPROJECT_API UAttackTreeNode : public UGenericGraphNode
{
	GENERATED_BODY()
public:
	UAttackTreeNode();

	const FAttackProperties& GetAttackProperties() const{ return AttackProperties; }
	//wrapper to make ExecuteAttack accessible without giving write access to all of AttackProperties contents 
	void ExecuteAttack(UWorld* WorldContext){ AttackProperties.Execute(WorldContext); }

#if WITH_EDITOR
	virtual FText GetNodeTitle() const override;
	virtual void SetNodeTitle(const FText& NewTitle) override;
	virtual FLinearColor GetBackgroundColor() const override;
#endif
protected:
	UPROPERTY(EditDefaultsOnly)
	FText AttackTitle;

	UPROPERTY(EditDefaultsOnly, Category = AttackProperties , meta=(ShowOnlyInnerProperties))
	FAttackProperties AttackProperties;
};
