// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttackTreeBaseNode.h"
#include "AttackTreeRootNode.generated.h"

/**
 * 
 */
UCLASS()
class MAPROJECT_API UAttackTreeRootNode : public UAttackTreeBaseNode
{
	GENERATED_BODY()
public:
	UAttackTreeRootNode();
	
	bool GetIsMainRootNode(const FString& Identifier = FString()) const;
	
#if WITH_EDITOR
	virtual void SetNodeTitle(const FText& NewTitle) override;
	virtual FText GetNodeTitle() const override;
#endif
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = NodeSettings)
	uint8 bIsMainRootNode:1;

	//an identifying name used when teleporting between root noes (has no effect otherwise)
	UPROPERTY(EditDefaultsOnly, Category = NodeSettings)
	FString JumpToIdentifier;
};
