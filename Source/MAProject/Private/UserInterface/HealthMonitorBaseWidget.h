// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthMonitorBaseWidget.generated.h"

class UProgressBar;
class UHealthMonitorBaseWidget;

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
class UHealthMonitorBaseWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	
	virtual void SetupInformation(uint32 NewMaxHealth, uint32 CurrentHealth, FSetupInformationKey);
	
	/// @brief Update health bar
	UFUNCTION()
	virtual void UpdateHealth(uint32 NewHealth, uint32 OldHealth);

protected:
	uint32 MaxHealth;
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;
};
