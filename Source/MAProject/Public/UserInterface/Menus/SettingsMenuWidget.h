// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsMenuWidget.generated.h"

class UToggleButton;
class UButton;
class UComboBoxString;
class USlider;

/**
 * 
 */
UCLASS()
class MAPROJECT_API USettingsMenuWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual bool Initialize() override;

protected:
	UPROPERTY()
	UGameUserSettings* GameUserSettings;
	
	UPROPERTY(meta = (BindWidget))
	UToggleButton* WindowedModeToggle;
	UPROPERTY(meta = (BindWidget))
	UButton* ApplySettingsButton;
	UPROPERTY(meta = (BindWidget))
	UButton* SetOptimalSettingsButton;

	UPROPERTY(meta = (BindWidget))
	UComboBoxString* DisplayResolutionDropdown;
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* FramerateLimitDropdown;
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* ScalabilityPresetDropdown;
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* ViewDistanceDropdown;
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* AntiAliasingDistanceDropdown;
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* PostProcessingDropdown;
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* ShadowsDropdown;
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* GlobalIlluminationDropdown;
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* ReflectionsDropdown;
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* TexturesDropdown;
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* EffectsDropdown;
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* FoliageDropdown;
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* ShadingDropdown;

	UPROPERTY(meta = (BindWidget))
	UButton* SaveGameButton;

	UPROPERTY(meta = (BindWidget))
	UToggleButton* PreferMostCenteredTargetToggle;
	UPROPERTY(meta = (BindWidget))
	USlider* ZoomSpeedSlider;

	void SetAllScalabilityIsEnabled(bool IsEnabled) const;
	
	void LoadCurrentFromSettings() const;
	void LoadCurrentScalabilityFromSettings(int32 Scalability) const;
	
	UFUNCTION()
	void SetOptimalSettings();
	UFUNCTION()
	void ApplySettings();
	UFUNCTION()
	void OnScalabilityPresetChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void SaveGame();
};
