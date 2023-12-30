// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttackProperties.h"
#include "Characters/Fighters/Attacks/AttackTree/AttackTreeEdge.h"
#include "Attacks.generated.h"


class UAttackTreeRootNode;
class UAttackNode;
class UAttackTree;
class UGenericGraphNode;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExecuteAttackDelegate, const FAttackProperties&, AttackProperties);
DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FOnCheckCanExecuteAttackDelegate, const FAttackProperties&, AttackProperties);

//this useless class is required for Attacks.generated.h to be generated from the UHT,
//which is somehow required for a working definition of the two delegates
USTRUCT()
struct FCompilationDummy final
{
	GENERATED_BODY();
};

struct FSetAttackTreeModeIdentifier
{
	friend class AFighterCharacter;
private:
	FSetAttackTreeModeIdentifier(){}
};


struct FAttacks
{
public:
	TDelegate<void(UAttackNode* IdentifiedNode, AttackIndex)> OnCdChanged;
	TDelegate<void()> OnModeChanged;
	FOnExecuteAttackDelegate OnExecuteAttack;
	FOnCheckCanExecuteAttackDelegate OnCheckCanExecuteAttack;
	
	FAttacks() : ComboExpirationTime(0.0), PendingAttackProperties(nullptr), AttackTree(nullptr), CurrentNode(nullptr){}

	FAttacks(UAttackTree const* AttackTree, UObject* Outer);

	bool HasPendingAttackProperties() const { return PendingAttackProperties != nullptr; }
	FAttackProperties const* GetPendingAttackProperties() const { return PendingAttackProperties; }
	void ClearPendingAttackPropertiesInternal(){ PendingAttackProperties = nullptr; }
	double GetComboExpirationTime() const { return ComboExpirationTime; }
	
	const UGenericGraphNode* GetCurrentNode(UWorld* WorldContext) const;
	const UGenericGraphNode* GetRootNode() const{ return GetRootNodeInternal(); }

	void SetModeIdentifier(const FString& ModeIdentifier, FSetAttackTreeModeIdentifier);

	UAttackNode* GetFirstNodeMatchingIndex(AttackIndex Index);
	/// Execute the attack by either continuing the current combo string or by starting a new string from the root node
	bool ExecuteAttack(AttackIndex Index, const AActor* PlayingInstance, UWorld* WorldContext);
	/// Execute the exact given attack
	bool ExecuteAttackFromNode(UAttackNode* NodeToExecute, const AActor* PlayingInstance, UWorld* WorldContext);

	void ForceSetCd(const FString& NodeIdentifier, float CdTime, bool ChangeBy);

	bool operator==(const FAttacks& Attacks) const;

protected:
	double ComboExpirationTime;
	FAttackProperties const* PendingAttackProperties;

	FString RootNodeIdentifier;
	TArray<UAttackTreeRootNode*> PreCastedRootNodes;
	TArray<UAttackNode*> PreCastedAttackNodes;
	UAttackTree* AttackTree;
	UGenericGraphNode* CurrentNode;

	UGenericGraphNode* FAttacks::GetRootNodeInternal() const;

	void ExecuteAttackInternal(UAttackNode* Node, const FAttackProperties& Properties, UWorld* WorldContext);

	FORCEINLINE bool HasExceededComboTime(UWorld* WorldContext) const;
};
