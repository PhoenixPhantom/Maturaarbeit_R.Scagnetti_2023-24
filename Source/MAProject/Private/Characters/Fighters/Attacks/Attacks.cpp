// Fill out your copyright notice in the Description page of Project Settings.


#include "Attacks.h"

#include "Characters/Fighters/Attacks/AttackTree/AttackTree.h"
#include "Characters/Fighters/Attacks/AttackTree/AttackTreeNode.h"

FAttacks::FAttacks(UAttackTree* AttackTree, UObject* Outer) : NodeAccessTime(-1.f)
{
	this->AttackTree = DuplicateObject(AttackTree, Outer);
	CurrentNode = CastChecked<UAttackTreeNode>(this->AttackTree->RootNodes[0]);
}

const FAttackProperties& FAttacks::GetLatestAttackProperties() const
{
	check(IsValid(CurrentNode));
	return CurrentNode->GetAttackProperties();
}

const UAttackTreeNode* FAttacks::GetCurrentNode(UWorld* WorldContext) const
{
	if(HasExceededComboTime(WorldContext))
	{
		return CastChecked<UAttackTreeNode>(GetRootNode());
	}
	return CurrentNode;
}

const UGenericGraphNode* FAttacks::GetRootNode() const
{
	//We assume there is only one root since this simplifies the process a lot
	return AttackTree->RootNodes[0];
}

bool FAttacks::ExecuteAttack(AttackIndex Index, UWorld* WorldContext)
{
	UAttackTreeNode* ResultingAttackNode = nullptr;
	if(!HasExceededComboTime(WorldContext))
	{
		for(UGenericGraphNode* ChildNode : CurrentNode->ChildrenNodes)
		{
			const UAttackTreeEdge* AttackTreeEdge = CastChecked<UAttackTreeEdge>(CurrentNode->GetEdge(ChildNode));
			if(AttackTreeEdge->IndexCondition == Index)
			{
				ResultingAttackNode = CastChecked<UAttackTreeNode>(ChildNode);
				break;
			}
		}
	}

	//we allow "jumping back" to the root node for two reasons:
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
				ResultingAttackNode = CastChecked<UAttackTreeNode>(ChildNode);
				break;
			}
		}
		if(ResultingAttackNode == nullptr) return false;
	}
	
	const FAttackProperties& AttackProperties = ResultingAttackNode->GetAttackProperties();

	if(AttackProperties.GetIsOnCd()) return false;
	if(OnCheckCanExecuteAttack.IsBound() && !OnCheckCanExecuteAttack.Execute(AttackProperties)) return false;

	CurrentNode = ResultingAttackNode;
	NodeAccessTime = WorldContext->RealTimeSeconds;
	CurrentNode->ExecuteAttack(WorldContext);
	OnExecuteAttack.Broadcast(AttackProperties);
	return true;
}

bool FAttacks::ExecuteAttackFromNode(UAttackTreeNode* NodeToExecute, UWorld* WorldContext)
{
	
	const FAttackProperties& AttackProperties = NodeToExecute->GetAttackProperties();

	if(!GetRootNode()->ChildrenNodes.Contains(NodeToExecute) &&
		(!HasExceededComboTime(WorldContext) && !CurrentNode->ChildrenNodes.Contains(NodeToExecute)))
	{
		checkNoEntry();
		return false;
	}
	if(AttackProperties.GetIsOnCd() || (OnCheckCanExecuteAttack.IsBound() && !OnCheckCanExecuteAttack.Execute(AttackProperties)))
	{
		checkNoEntry();
		return false;
	}

	CurrentNode = NodeToExecute;
	NodeAccessTime = WorldContext->RealTimeSeconds;
	CurrentNode->ExecuteAttack(WorldContext);
	OnExecuteAttack.Broadcast(AttackProperties);
	return true;
}

bool FAttacks::operator==(const FAttacks& Attacks) const
{
	return NodeAccessTime == Attacks.NodeAccessTime && AttackTree == Attacks.AttackTree && CurrentNode == Attacks.CurrentNode;
}

bool FAttacks::HasExceededComboTime(UWorld* WorldContext) const
{
	return WorldContext->RealTimeSeconds - NodeAccessTime > GetLatestAttackProperties().MaxComboTime;
}
