// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Attacks/AttackTree/AttackTree.h"

#include "Characters/Fighters/Attacks/AttackTree/AttackTreeBaseNode.h"
#include "Characters/Fighters/Attacks/AttackTree/AttackTreeEdge.h"

UAttackTree::UAttackTree() : bShowAsPlayerTree(false)
{
	NodeType = UAttackTreeBaseNode::StaticClass();
	EdgeType = UAttackTreeEdge::StaticClass();

	Name = "Attack Tree";
}
