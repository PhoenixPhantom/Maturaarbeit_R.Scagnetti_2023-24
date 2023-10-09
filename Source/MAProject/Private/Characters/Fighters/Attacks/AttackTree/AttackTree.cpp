// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Attacks/AttackTree/AttackTree.h"

#include "Characters/Fighters/Attacks/AttackTree/AttackTreeEdge.h"
#include "Characters/Fighters/Attacks/AttackTree/AttackTreeNode.h"

UAttackTree::UAttackTree() : bShowAsPlayerTree(false)
{
	NodeType = UAttackTreeNode::StaticClass();
	EdgeType = UAttackTreeEdge::StaticClass();

	Name = "Attack Tree";
}
