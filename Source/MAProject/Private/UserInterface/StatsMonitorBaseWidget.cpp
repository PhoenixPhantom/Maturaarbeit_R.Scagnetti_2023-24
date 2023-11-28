// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/StatsMonitorBaseWidget.h"

#include "Components/ProgressBar.h"

void UStatsMonitorBaseWidget::SetupInformation(const TMaxedValue<int32, float>& Health,
	const TMaxedValue<int32, float>& Toughness, FSetupInformationKey)
{
	MaxHealth = Health.Maximum.GetResulting();
	UpdateHealth(Health.Current, Health.Current);
	MaxToughness = Toughness.Maximum.GetResulting();
	UpdateToughness(Toughness.Current, Toughness.Current);
}

void UStatsMonitorBaseWidget::UpdateToughness(int32 NewToughness, int32 OldToughness)
{
	check(IsValid(ToughnessBar));
	ToughnessBar->SetPercent(static_cast<float>(NewToughness) /
	static_cast<float>(MaxToughness));
}

void UStatsMonitorBaseWidget::UpdateMaxToughness(int32 CurrentToughness, int32 NewMaxToughness)
{
	check(IsValid(ToughnessBar));
	MaxToughness = NewMaxToughness;
	ToughnessBar->SetPercent(static_cast<float>(CurrentToughness) /
	static_cast<float>(MaxToughness));
}

void UStatsMonitorBaseWidget::SetHealthVisibility(ESlateVisibility NewVisibility)
{
	HealthBar->SetVisibility(NewVisibility);
	ToughnessBar->SetVisibility(NewVisibility);
}


void UStatsMonitorBaseWidget::UpdateHealthInternal(int32 NewHealth, int32 OldHealth)
{
	check(IsValid(HealthBar));
	if(NewHealth == 0) SetHealthVisibility(ESlateVisibility::Hidden);
	else if(HealthBar->GetVisibility() == ESlateVisibility::Hidden) SetHealthVisibility(ESlateVisibility::Visible);
	HealthBar->SetPercent(static_cast<float>(NewHealth) /
	static_cast<float>(MaxHealth));
}

void UStatsMonitorBaseWidget::UpdateMaxHealthInternal(int32 CurrentHealth, int32 NewMaxHealth)
{
	check(IsValid(HealthBar));
	MaxHealth = NewMaxHealth;
	HealthBar->SetPercent(static_cast<float>(CurrentHealth) /
	static_cast<float>(MaxHealth));
}
