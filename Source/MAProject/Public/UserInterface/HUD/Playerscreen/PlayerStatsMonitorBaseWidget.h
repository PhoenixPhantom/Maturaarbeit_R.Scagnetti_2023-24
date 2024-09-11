// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UserInterface/StatsMonitorBaseWidget.h"
#include "PlayerStatsMonitorBaseWidget.generated.h"

class UImage;
class UTextBlock;
/**
 * 
 */
UCLASS()
class MAPROJECT_API UPlayerStatsMonitorBaseWidget : public UStatsMonitorBaseWidget
{
	GENERATED_BODY()

public:
	UPlayerStatsMonitorBaseWidget();

	void SetTotalSkillCdTime(float CdTime);
	void SetTotalUltimateCdTime(float CdTime);
	void SetSkillTimer(FTimerHandle SkillTimerHandle);
	void SetUltimateTimer(FTimerHandle UltimateTimerHandle);
	UImage* GetFirstAvailableImage();
	UImage* GetFirstUnconnectedImage();

protected:
	float SkillTotalCd;
	float UltimateTotalCd;
	FTimerHandle SkillCdHandle;
	FTimerHandle UltimateCdHandle;

	TArray<UImage*> StatusEffectMarkers;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SkillCdTime;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* UltimateCdTime;
		
	UPROPERTY(meta = (BindWidget))
	UImage* SkillCdPercentage;

	UPROPERTY(meta = (BindWidget))
	UImage* UltimateCdPercentage;
	
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void UpdateHealthInternal(int32 NewHealth, int32 OldHealth) override;
	virtual void UpdateMaxHealthInternal(int32 CurrentHealth, int32 NewMaxHealth) override;
	void SetSkillCdTime(float CdTime);
	void SetUltimateCdTime(float CdTime);

	UFUNCTION(BlueprintCallable)
	void RegisterStatusEffectMarker(const TArray<UImage*>& Markers){ StatusEffectMarkers.Append(Markers); }
};
