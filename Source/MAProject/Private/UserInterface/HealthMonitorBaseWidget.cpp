// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/HealthMonitorBaseWidget.h"

#include "Components/ProgressBar.h"

void UHealthMonitorBaseWidget::SetupInformation(uint32 NewMaxHealth, uint32 CurrentHealth, FSetupInformationKey) {
	MaxHealth = NewMaxHealth;
	UpdateHealth(CurrentHealth, CurrentHealth);
}

void UHealthMonitorBaseWidget::UpdateHealth(uint32 NewHealth, uint32 OldHealth)
{
	check(IsValid(HealthBar));
	if(NewHealth == 0) HealthBar->SetVisibility(ESlateVisibility::Hidden);
	HealthBar->SetPercent(static_cast<float>(NewHealth) /
	static_cast<float>(MaxHealth));
}