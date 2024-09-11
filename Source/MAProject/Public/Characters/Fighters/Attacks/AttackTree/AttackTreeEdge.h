// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericGraphEdge.h"
#include "AttackTreeEdge.generated.h"

class UAttackNode;


typedef int32 AttackIndex;

enum EAttackType : AttackIndex
{
	AttackType_Light,
	AttackType_Heavy,
	AttackType_Skill,
	AttackType_Ultimate
};

/**
 * 
 */
UCLASS()
class MAPROJECT_API UAttackTreeEdge : public UGenericGraphEdge
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	int32 IndexCondition;

protected:
	virtual FText GetNodeTitle() const override;
};
