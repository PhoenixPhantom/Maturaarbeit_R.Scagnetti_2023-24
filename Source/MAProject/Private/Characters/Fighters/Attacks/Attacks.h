// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttackProperties.h"
#include "Characters/Fighters/Attacks/AttackTree/AttackTreeEdge.h"
#include "Attacks.generated.h"


class UAttackTreeNode;
class UAttackTree;
class UGenericGraphNode;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExecuteAttackDelegate, const FAttackProperties&, AttackProperties);
DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FOnCheckCanExecuteAttackDelegate, const FAttackProperties&, AttackProperties);

//this useless class is required for Attacks.generated.h to be generated from the UHT,
//which is required for a working definition of the two delegates
USTRUCT()
struct FCompilationDummy
{
	GENERATED_BODY();
};


struct FAttacks
{
public:
	FOnExecuteAttackDelegate OnExecuteAttack;
	FOnCheckCanExecuteAttackDelegate OnCheckCanExecuteAttack;
	
	FAttacks() : NodeAccessTime(-1.f), AttackTree(nullptr), CurrentNode(nullptr){}
	FAttacks(UAttackTree* AttackTree, UObject* Outer);

	const FAttackProperties& GetLatestAttackProperties() const;
	const UAttackTreeNode* GetCurrentNode(UWorld* WorldContext) const;
	const UGenericGraphNode* GetRootNode() const;

	/// Execute the attack by either continuing the current combo string or by starting a new string from the root node
	bool ExecuteAttack(AttackIndex Index, UWorld* WorldContext);
	bool ExecuteAttackFromNode(UAttackTreeNode* NodeToExecute, UWorld* WorldContext);

	bool operator==(const FAttacks& Attacks) const;
protected:
	double NodeAccessTime;
	
	UAttackTree* AttackTree;
	UAttackTreeNode* CurrentNode;

	FORCEINLINE bool HasExceededComboTime(UWorld* WorldContext) const;
};
