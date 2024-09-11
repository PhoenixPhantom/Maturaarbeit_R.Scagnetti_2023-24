// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Attacks/AttackTree/AttackTreeEdge.h"

#include "Characters/Fighters/Attacks/AttackTree/AttackTree.h"

#define LOCTEXT_NAMESPACE "AttackTreeEdge"

FText UAttackTreeEdge::GetNodeTitle() const
{
	if(UAttackTree* AttackTree = Cast<UAttackTree>(Graph); IsValid(AttackTree) && AttackTree->GetShowAsPlayerTree())
	{
		switch(IndexCondition)
		{
			case AttackType_Light: return LOCTEXT("AttackTreeEdge", "L");
			case AttackType_Heavy: return LOCTEXT("AttackTreeEdge", "H");
			case AttackType_Skill: return LOCTEXT("AttackTreeEdge", "S");
			case AttackType_Ultimate: return LOCTEXT("AttackTreeEdge", "U");
			default: return LOCTEXT("AttackTreeEdge", "?");
		}
		
	}
	return FText::FromString(FString::FromInt(IndexCondition));
}

#undef LOCTEXT_NAMESPACE
