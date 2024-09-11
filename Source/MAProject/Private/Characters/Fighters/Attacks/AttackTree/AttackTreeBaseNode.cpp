// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Attacks/AttackTree/AttackTreeBaseNode.h"

#include "Characters/Fighters/Attacks/AttackTree/AttackTree.h"

#define  LOCTEXT_NAMESPACE "AttackTreeBaseNode"

UAttackTreeBaseNode::UAttackTreeBaseNode()
{
#if WITH_EDITORONLY_DATA
	CompatibleGraphType = UAttackTree::StaticClass();
	ContextMenuName = LOCTEXT("ContextMenuName", "Base Node (do not use)");
#endif
}

#undef LOCTEXT_NAMESPACE