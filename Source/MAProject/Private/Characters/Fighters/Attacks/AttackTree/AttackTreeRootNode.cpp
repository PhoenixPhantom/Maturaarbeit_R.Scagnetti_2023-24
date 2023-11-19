// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Attacks/AttackTree/AttackTreeRootNode.h"

#define  LOCTEXT_NAMESPACE "AttackTreeRootNode"

UAttackTreeRootNode::UAttackTreeRootNode(): bIsMainRootNode(false)
{
#if WITH_EDITORONLY_DATA
	ContextMenuName = LOCTEXT("ContextMenuName", "Root Node");
#endif
}

bool UAttackTreeRootNode::GetIsMainRootNode(const FString& Identifier) const
{
	return Identifier.IsEmpty() ? bIsMainRootNode : JumpToIdentifier == Identifier;
}


#if WITH_EDITOR
void UAttackTreeRootNode::SetNodeTitle(const FText& NewTitle)
{
	JumpToIdentifier = NewTitle.ToString();
}

FText UAttackTreeRootNode::GetNodeTitle() const
{
	if(JumpToIdentifier.Contains("root", ESearchCase::IgnoreCase))
	{
		return LOCTEXT("NodeFailureDescription", "INVALID NAME: SET bIsMainRootNode INSTEAD");
	}
	return FText::FromString(JumpToIdentifier + (bIsMainRootNode ? " (root)" : " (secondary root)"));
}
#endif

#undef LOCTEXT_NAMESPACE