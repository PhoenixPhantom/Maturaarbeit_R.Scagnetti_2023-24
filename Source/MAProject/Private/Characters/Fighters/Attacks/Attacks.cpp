// Fill out your copyright notice in the Description page of Project Settings.


#include "Attacks.h"

#include "Characters/Fighters/Attacks/AttackTree/AttackTree.h"
#include "Characters/Fighters/Attacks/AttackTree/AttackNode.h"
#include "Characters/Fighters/Attacks/AttackTree/AttackTreeRootNode.h"

FAttacks::FAttacks(UAttackTree const* AttackTree, UObject* Outer) : ComboExpirationTime(-1.0), PendingAttackProperties(nullptr)
{
	this->AttackTree = DuplicateObject(AttackTree, Outer);
	for(UGenericGraphNode* GraphNode : this->AttackTree->RootNodes)
	{
		PreCastedRootNodes.Add(CastChecked<UAttackTreeRootNode>(GraphNode));
	}
	for(UGenericGraphNode* GraphNode : this->AttackTree->AllNodes)
	{
		UAttackNode* AttackNode = Cast<UAttackNode>(GraphNode);
		if(IsValid(AttackNode))
		{
			PreCastedAttackNodes.Add(AttackNode);
		}
	}
	CurrentNode = GetRootNodeInternal();
}

const UGenericGraphNode* FAttacks::GetCurrentNode(UWorld* WorldContext) const
{
	if(HasExceededComboTime(WorldContext))
	{
		return GetRootNode();
	}
	return CurrentNode;
}

void FAttacks::SetModeIdentifier(const FString& ModeIdentifier, FSetAttackTreeModeIdentifier)
{
	RootNodeIdentifier = ModeIdentifier;
	CurrentNode = GetRootNodeInternal();
	ComboExpirationTime = -1.0;
	// ReSharper disable once CppExpressionWithoutSideEffects
	OnModeChanged.ExecuteIfBound();
}

bool FAttacks::ExecuteAttack(AttackIndex Index, const AActor* PlayingInstance, UWorld* WorldContext)
{
	UAttackNode* ResultingAttackNode = nullptr;
	if(!HasExceededComboTime(WorldContext))
	{
		for(UGenericGraphNode* ChildNode : CurrentNode->ChildrenNodes)
		{
			const UAttackTreeEdge* AttackTreeEdge = CastChecked<UAttackTreeEdge>(CurrentNode->GetEdge(ChildNode));
			if(AttackTreeEdge->IndexCondition == Index)
			{
				ResultingAttackNode = CastChecked<UAttackNode>(ChildNode);
				break;
			}
		}
	}

	//we allow "jumping back" to the root node for one of two reasons:
	//    1. the combo time for the current attack string has been exceeded
	//    2. the received input is not part of the current attack string
	if(ResultingAttackNode == nullptr)
	{
		const UGenericGraphNode* RootNode = GetRootNode();
		for(UGenericGraphNode* ChildNode : RootNode->ChildrenNodes)
		{
			const UAttackTreeEdge* AttackTreeEdge = CastChecked<UAttackTreeEdge>(RootNode->GetEdge(ChildNode));
			if(AttackTreeEdge->IndexCondition == Index)
			{
				ResultingAttackNode = CastChecked<UAttackNode>(ChildNode);
				break;
			}
		}
		if(ResultingAttackNode == nullptr) return false;
	}
	
	const FAttackProperties& AttackProperties = ResultingAttackNode->GetAttackProperties(PlayingInstance);

	if(ResultingAttackNode->GetIsOnCd()) return false;
	if(OnCheckCanExecuteAttack.IsBound() && !OnCheckCanExecuteAttack.Execute(AttackProperties)) return false;

	ExecuteAttackInternal(ResultingAttackNode, AttackProperties, WorldContext);
	return true;
}

bool FAttacks::ExecuteAttackFromNode(UAttackNode* NodeToExecute, const AActor* PlayingInstance, UWorld* WorldContext)
{
	
	const FAttackProperties& AttackProperties = NodeToExecute->GetAttackProperties(PlayingInstance);

	if(!GetRootNode()->ChildrenNodes.Contains(NodeToExecute) &&
		(!HasExceededComboTime(WorldContext) && !CurrentNode->ChildrenNodes.Contains(NodeToExecute)))
	{
		checkNoEntry();
		return false;
	}
	if(NodeToExecute->GetIsOnCd() || (OnCheckCanExecuteAttack.IsBound() && !OnCheckCanExecuteAttack.Execute(AttackProperties)))
	{
		checkNoEntry();
		return false;
	}
	
	ExecuteAttackInternal(NodeToExecute, AttackProperties, WorldContext);
	return true;
}

void FAttacks::ForceSetCd(const FString& NodeIdentifier, float RemainingCd)
{
	UAttackNode** FoundNode = PreCastedAttackNodes.FindByPredicate([NodeIdentifier](UAttackNode* AttackNode)
	{
		return AttackNode->CanBeCalled(NodeIdentifier);
	});

	if(FoundNode == nullptr) return;
	(*FoundNode)->ForceSetCd(RemainingCd);
}

void FAttacks::ForceChangeCdBy(const FString& NodeIdentifier, float CdChange)
{
	UAttackNode** FoundNode = PreCastedAttackNodes.FindByPredicate([NodeIdentifier](UAttackNode* AttackNode)
	{
		return AttackNode->CanBeCalled(NodeIdentifier);
	});

	if(FoundNode == nullptr) return;
	(*FoundNode)->ForceSetCd((*FoundNode)->CdTimeRemaining() + CdChange);
}

bool FAttacks::operator==(const FAttacks& Attacks) const
{
	return ComboExpirationTime == Attacks.ComboExpirationTime && AttackTree == Attacks.AttackTree &&
		CurrentNode == Attacks.CurrentNode;
}

UGenericGraphNode* FAttacks::GetRootNodeInternal() const
{
	for(UAttackTreeRootNode* RootNode : PreCastedRootNodes)
	{
		if(RootNode->GetIsMainRootNode(RootNodeIdentifier))
		{
			return RootNode;
		}
	}
	checkNoEntry();
	return nullptr;
}

void FAttacks::ExecuteAttackInternal(UAttackNode* Node, const FAttackProperties& Properties, UWorld* WorldContext)
{
	CurrentNode = Node;
	ComboExpirationTime = WorldContext->RealTimeSeconds + Properties.MaxComboTime;
	PendingAttackProperties = &Properties;
	OnExecuteAttack.Broadcast(Properties);
	Node->Execute();
}

bool FAttacks::HasExceededComboTime(UWorld* WorldContext) const
{
	return WorldContext->RealTimeSeconds > ComboExpirationTime;
}
