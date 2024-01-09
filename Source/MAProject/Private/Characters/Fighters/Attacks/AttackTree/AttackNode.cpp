// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Attacks/AttackTree/AttackNode.h"

#include "Utility/Stats/StatusEffect.h"


#define LOCTEXT_NAMESPACE "AttackTreeNode"

FAttackPropertiesNode::FAttackPropertiesNode()
{
}

FAttackPropertiesNode::FAttackPropertiesNode(const FAttackPropertiesNode& Properties) : Super(Properties),
	AttackTitle(Properties.AttackTitle)
{
}

FAttackPropertiesNode::FAttackPropertiesNode(const FAttackProperties& Properties) : Super(Properties)
{
}

bool FAttackPropertiesNode::operator==(const FAttackPropertiesNode& AttackProperties) const
{
	return Super::operator==(AttackProperties) && AttackTitle == AttackProperties.AttackTitle;
}

FAttackPropertiesNodeAdditional::FAttackPropertiesNodeAdditional()
{
}

FAttackPropertiesNodeAdditional::FAttackPropertiesNodeAdditional(const FAttackPropertiesNodeAdditional& Properties) :
	Super(Properties), ExecutionCondition(Properties.ExecutionCondition)
{
}

FAttackPropertiesNodeAdditional::FAttackPropertiesNodeAdditional(const FAttackPropertiesNode& Properties) :
	Super(Properties)
{
}

FAttackPropertiesNodeAdditional::FAttackPropertiesNodeAdditional(const FAttackProperties& Properties) :
	Super(Properties)
{
}

bool FAttackPropertiesNodeAdditional::operator==(const FAttackPropertiesNode& AttackProperties) const
{
	return Super::operator==(AttackProperties);
}

UAttackNode::UAttackNode(): bIsOnCd(false)
{
#if WITH_EDITORONLY_DATA
	ContextMenuName = LOCTEXT("ContextMenuName", "Attack Node");
	BackgroundColor = FColor(1.f, 0.f, 0.f);
#endif
}

const FAttackProperties& UAttackNode::GetAttackProperties(const AActor* PlayingInstance) const
{
	if(IsValid(PlayingInstance) && !AdditionalAttacks.IsEmpty())
	{
		TArray<UActorComponent*> ActiveStatusEffects;
		PlayingInstance->GetComponents(UStatusEffect::StaticClass(), ActiveStatusEffects);

		for(const FAttackPropertiesNodeAdditional& AttackPropertiesNodeAdditional : AdditionalAttacks)
		{
			if(ActiveStatusEffects.FindByPredicate(
				[ExecutionCondition = AttackPropertiesNodeAdditional.ExecutionCondition]
				(UActorComponent* StatusEffect)
			{
				return StatusEffect->IsA(ExecutionCondition);
			}) != nullptr)
			{
				return AttackPropertiesNodeAdditional;
			}
		}
	}
	return AttackProperties;
}

bool UAttackNode::CanBeCalled(const FString& NodeName) const
{
	return AttackProperties.AttackTitle == NodeName;
}

#if WITH_EDITOR
FText UAttackNode::GetNodeTitle() const
{
	FString AdditionalAttackSpecifiers;
	if(!AdditionalAttacks.IsEmpty())
	{
		AdditionalAttackSpecifiers.Append(" (Default");
		for(const FAttackPropertiesNode& AttackPropertiesNode : AdditionalAttacks)
		{
			AdditionalAttackSpecifiers.Append(", " + AttackPropertiesNode.AttackTitle);
		}
		AdditionalAttackSpecifiers.Append(")");
	}
	return FText::FromString(AttackProperties.AttackTitle + AdditionalAttackSpecifiers);
}

void UAttackNode::SetNodeTitle(const FText& NewTitle)
{
	FString UnusedTitleFragment = NewTitle.ToString();
	if(!AdditionalAttacks.IsEmpty())
	{
		if(UnusedTitleFragment.RemoveFromEnd(")"))
		{
			for(FAttackPropertiesNode& AttackPropertiesNode : AdditionalAttacks)
			{
				const int32 RelevantDivisor = UnusedTitleFragment.Find(", ", ESearchCase::IgnoreCase, ESearchDir::FromEnd);
				if(RelevantDivisor == INDEX_NONE) continue;
				AttackPropertiesNode.AttackTitle = UnusedTitleFragment.RightChop(RelevantDivisor + 2);
				UnusedTitleFragment.RemoveFromEnd(UnusedTitleFragment.RightChop(RelevantDivisor));
			}
			UnusedTitleFragment.RemoveFromEnd(" (Default");
		}
	}
	AttackProperties.AttackTitle = UnusedTitleFragment;
}

FLinearColor UAttackNode::GetBackgroundColor() const
{
	return BackgroundColor;
}
#endif



void UAttackNode::Execute()
{
	const float ActualCdTime = AttackProperties.GetTotalCdTime();
	if(ActualCdTime <= 0.f) return;
	bIsOnCd = true;
	GetOuter()->GetWorld()->GetTimerManager().SetTimer(CdHandle,
		[this](){ bIsOnCd = false; },ActualCdTime, false);
}

void UAttackNode::ForceSetCd(float DesiredCd)
{
	if(GetOuter()->GetWorld()->GetTimerManager().IsTimerActive(CdHandle))
	{
		GetOuter()->GetWorld()->GetTimerManager().ClearTimer(CdHandle);
	}
	
	if(DesiredCd <= 0.f)
	{
		bIsOnCd = false;
	}
	else
	{
		bIsOnCd = true;
		GetOuter()->GetWorld()->GetTimerManager().SetTimer(CdHandle,
		[this](){ bIsOnCd = false; },DesiredCd, false);
	}
}

float UAttackNode::CdTimeElapsed() const
{
	if(!bIsOnCd) return -1.f;
	return GetOuter()->GetWorld()->GetTimerManager().GetTimerElapsed(CdHandle);
}

float UAttackNode::CdTimeRemaining() const
{
	if(!bIsOnCd) return -1.f;
	return GetOuter()->GetWorld()->GetTimerManager().GetTimerRemaining(CdHandle);
}
#undef LOCTEXT_NAMESPACE