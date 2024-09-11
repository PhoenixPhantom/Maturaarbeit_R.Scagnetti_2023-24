// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/HUD/Playerscreen/PlayerStatsMonitorBaseWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

#define LOCTEXT_NAMESPACE "PlayerStatsMonitorBaseWidget"


UPlayerStatsMonitorBaseWidget::UPlayerStatsMonitorBaseWidget(): SkillTotalCd(0), UltimateTotalCd(0),
	HealthText(nullptr), SkillCdTime(nullptr), UltimateCdTime(nullptr), SkillCdPercentage(nullptr),
	UltimateCdPercentage(nullptr)
{
}

void UPlayerStatsMonitorBaseWidget::SetTotalSkillCdTime(float CdTime)
{
	SkillTotalCd = CdTime;
	if(!SkillCdHandle.IsValid()) SetSkillCdTime(-1.f);
}

void UPlayerStatsMonitorBaseWidget::SetTotalUltimateCdTime(float CdTime)
{
	UltimateTotalCd = CdTime;
	if(!UltimateCdHandle.IsValid()) SetUltimateCdTime(-1.f);
}

void UPlayerStatsMonitorBaseWidget::SetSkillTimer(FTimerHandle SkillTimerHandle)
{
	SkillCdHandle = SkillTimerHandle;
}

void UPlayerStatsMonitorBaseWidget::SetUltimateTimer(FTimerHandle UltimateTimerHandle)
{
	UltimateCdHandle = UltimateTimerHandle;
}

UImage* UPlayerStatsMonitorBaseWidget::GetFirstAvailableImage()
{
	for(UImage* Image : StatusEffectMarkers)
	{
		if(Image->GetVisibility() != ESlateVisibility::Hidden) continue;
		return Image;
	}
	return nullptr;
}

UImage* UPlayerStatsMonitorBaseWidget::GetFirstUnconnectedImage()
{
	bool FoundGap = false;
	for(UImage* Image : StatusEffectMarkers)
	{
		if(!FoundGap)
		{
			if(Image->GetVisibility() == ESlateVisibility::Hidden) FoundGap = true;
			continue;
		}
		
		if(Image->GetVisibility() == ESlateVisibility::Hidden) continue;
		return Image;
	}
	return nullptr;
}

void UPlayerStatsMonitorBaseWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if(SkillCdHandle.IsValid())
	{
		SetSkillCdTime(GetWorld()->GetTimerManager().GetTimerRemaining(SkillCdHandle));
	}
	if(UltimateCdHandle.IsValid())
	{
		SetUltimateCdTime(GetWorld()->GetTimerManager().GetTimerRemaining(UltimateCdHandle));
	}
}

void UPlayerStatsMonitorBaseWidget::UpdateHealthInternal(int32 NewHealth, int32 OldHealth)
{
	Super::UpdateHealthInternal(NewHealth, OldHealth);
	HealthText->SetText(FText::Format(LOCTEXT("ProtagonistHealthVal", "{0}/{1}"), NewHealth, MaxHealth));
}

void UPlayerStatsMonitorBaseWidget::UpdateMaxHealthInternal(int32 CurrentHealth, int32 NewMaxHealth)
{
	Super::UpdateMaxHealthInternal(CurrentHealth, NewMaxHealth);
	HealthText->SetText(FText::Format(LOCTEXT("ProtagonistHealthVal", "{0}/{1}"), CurrentHealth, MaxHealth));
}

void UPlayerStatsMonitorBaseWidget::SetSkillCdTime(float CdTime)
{
	if(CdTime <= 0)
	{
		SkillCdHandle.Invalidate();
		SkillCdTime->SetVisibility(ESlateVisibility::Hidden);
		SkillCdPercentage->GetDynamicMaterial()->SetScalarParameterValue("Alpha", 1.f);
		return;
	}
	if(SkillCdTime->GetVisibility() == ESlateVisibility::Hidden)
	{
		SkillCdTime->SetVisibility(ESlateVisibility::Visible);
	}
	SkillCdTime->SetText(FText::FromString(FString::SanitizeFloat(floor(CdTime * 10) * 0.1, 1)));
	SkillCdPercentage->GetDynamicMaterial()->SetScalarParameterValue("Alpha", 1.f - CdTime/SkillTotalCd);
}

void UPlayerStatsMonitorBaseWidget::SetUltimateCdTime(float CdTime)
{
	if(CdTime <= 0)
	{
		UltimateCdHandle.Invalidate();
		UltimateCdTime->SetVisibility(ESlateVisibility::Hidden);
		UltimateCdPercentage->GetDynamicMaterial()->SetScalarParameterValue("Alpha", 1.f);
		return;
	}
	if(UltimateCdTime->GetVisibility() == ESlateVisibility::Hidden)
	{
		UltimateCdTime->SetVisibility(ESlateVisibility::Visible);
	}
	UltimateCdTime->SetText(FText::FromString(FString::SanitizeFloat(floor(CdTime * 10) * 0.1, 1)));
	UltimateCdPercentage->GetDynamicMaterial()->SetScalarParameterValue("Alpha", 1.f - CdTime/UltimateTotalCd);
}


#undef LOCTEXT_NAMESPACE
