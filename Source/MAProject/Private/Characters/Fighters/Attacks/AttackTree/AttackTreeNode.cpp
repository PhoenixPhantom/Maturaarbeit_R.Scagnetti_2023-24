// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Attacks/AttackTree/AttackTreeNode.h"
#include "Characters/Fighters/Attacks/AttackTree/AttackTree.h"


#define LOCTEXT_NAMESPACE "AttackTreeNode"

UAttackTreeNode::UAttackTreeNode()
{
#if WITH_EDITORONLY_DATA
	CompatibleGraphType = UAttackTree::StaticClass();
	ContextMenuName = LOCTEXT("ContextMenuName", "AttackTreeNode");
#endif
}
#if WITH_EDITOR
FText UAttackTreeNode::GetNodeTitle() const
{
	return AttackTitle;
}

void UAttackTreeNode::SetNodeTitle(const FText& NewTitle)
{
	AttackTitle = NewTitle;
}

FLinearColor UAttackTreeNode::GetBackgroundColor() const
{
	return BackgroundColor;
}
#endif
#undef LOCTEXT_NAMESPACE