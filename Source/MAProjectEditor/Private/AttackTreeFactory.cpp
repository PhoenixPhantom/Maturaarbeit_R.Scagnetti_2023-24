// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackTreeFactory.h"
#include "Characters/Fighters/Attacks/AttackTree/AttackTree.h"

#define LOCTEXT_NAMESPACE "AttackTreeFactory"

UAttackTreeFactory::UAttackTreeFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UAttackTree::StaticClass();
}

UObject* UAttackTreeFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags,
	UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UObject>(InParent, Class, Name, Flags | RF_Transactional);
}

FText UAttackTreeFactory::GetDisplayName() const
{
	return LOCTEXT("FactoryName", "Attack Tree");
}

FString UAttackTreeFactory::GetDefaultNewAssetName() const
{
	return "AttackTree";
}

#undef LOCTEXT_NAMESPACE