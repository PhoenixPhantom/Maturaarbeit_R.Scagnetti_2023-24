// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Utility/Stats/GeneralStats.h"
#include "StatsMonitorBaseWidget.generated.h"

class UProgressBar;
class UStatsMonitorBaseWidget;

struct FSetupInformationKey final
{
	friend class AOpponentCharacter;
	friend class APlayerCharacter;
	friend class AFighterCharacter;
private:
	FSetupInformationKey(){}
};


/**
 * 
 */
UCLASS()
class UStatsMonitorBaseWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	
	virtual void SetupInformation(const TMaxedValue<int32, float>& Health, const TMaxedValue<int32, float>& Toughness, FSetupInformationKey);
	
	/// @brief Update health bar
	UFUNCTION()
	void UpdateHealth(int32 NewHealth, int32 OldHealth){ UpdateHealthInternal(NewHealth, OldHealth); }
	UFUNCTION()
	void UpdateMaxHealth(int32 CurrentHealth, int32 NewMaxHealth){ UpdateMaxHealthInternal(CurrentHealth, NewMaxHealth); }
	
	/// @brief Update toughness bar
	UFUNCTION()
	virtual void UpdateToughness(int32 NewToughness, int32 OldToughness);
	UFUNCTION()
	virtual void UpdateMaxToughness(int32 CurrentToughness, int32 NewMaxToughness);

protected:
	int32 MaxHealth;
	int32 MaxToughness;
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;
	UPROPERTY(meta = (BindWidget))
	UProgressBar* ToughnessBar;

	void SetHealthVisibility(ESlateVisibility NewVisibility);
	virtual void UpdateHealthInternal(int32 NewHealth, int32 OldHealth);
	virtual void UpdateMaxHealthInternal(int32 CurrentHealth, int32 NewMaxHealth);
};
