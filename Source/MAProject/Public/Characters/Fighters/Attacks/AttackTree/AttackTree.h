// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericGraph.h"
#include "AttackTree.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class MAPROJECT_API UAttackTree : public UGenericGraph
{
	GENERATED_BODY()
public:
	UAttackTree();
	bool GetShowAsPlayerTree() const { return bShowAsPlayerTree; };
protected:
	UPROPERTY(EditDefaultsOnly)
	bool bShowAsPlayerTree;
};
