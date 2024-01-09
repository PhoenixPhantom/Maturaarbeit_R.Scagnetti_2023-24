// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/Menus/SettingsMenuWidget.h"

#include "Characters/Fighters/Player/CustomGameState.h"
#include "Characters/Fighters/Player/PlayerPartyController.h"
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UserInterface/ToggleButton.h"

bool USettingsMenuWidget::Initialize()
{
	const bool bInitializeReturn = Super::Initialize();

	GameUserSettings = UGameUserSettings::GetGameUserSettings();
	GameUserSettings->LoadSettings();
	
	if(IsValid(DisplayResolutionDropdown))
	{
		//Set the available resolutions
		DisplayResolutionDropdown->ClearOptions();
		TArray<FIntPoint> AvailableResolutions;
		UKismetSystemLibrary::GetSupportedFullscreenResolutions(AvailableResolutions);
		for(const FIntPoint& Resolution : AvailableResolutions)
		{
			DisplayResolutionDropdown->AddOption(FString::FromInt(Resolution.X) + "x" + FString::FromInt(Resolution.Y));
		}
	
		//if the user changed the monitor and we have a fullscreen mode, we force the highest supported resolution
		//(so there are no wired scaling issues)
		//TODO: move to general startup (not options menu)
		if(!AvailableResolutions.Contains(GameUserSettings->GetScreenResolution()) &&
			GameUserSettings->GetFullscreenMode() == EWindowMode::Fullscreen)
		{
			GameUserSettings->SetScreenResolution(AvailableResolutions.Last());
		}
	}

	LoadCurrentFromSettings();
	
	//Bind the reaction functions
	
	if(IsValid(ScalabilityPresetDropdown))
		ScalabilityPresetDropdown->OnSelectionChanged.AddDynamic(this, &USettingsMenuWidget::OnScalabilityPresetChanged);
	if(IsValid(SetOptimalSettingsButton))
		SetOptimalSettingsButton->OnClicked.AddDynamic(this, &USettingsMenuWidget::SetOptimalSettings);
	if(IsValid(ApplySettingsButton))
		ApplySettingsButton->OnClicked.AddDynamic(this, &USettingsMenuWidget::ApplySettings);

	if(IsValid(SaveGameButton))
		SaveGameButton->OnClicked.AddDynamic(this, &USettingsMenuWidget::SaveGame);
	
	return bInitializeReturn;
}

void USettingsMenuWidget::SetAllScalabilityIsEnabled(bool IsEnabled) const
{
	ViewDistanceDropdown->SetIsEnabled(IsEnabled);
	AntiAliasingDistanceDropdown->SetIsEnabled(IsEnabled);
	PostProcessingDropdown->SetIsEnabled(IsEnabled);
	ShadowsDropdown->SetIsEnabled(IsEnabled);
	GlobalIlluminationDropdown->SetIsEnabled(IsEnabled);
	ReflectionsDropdown->SetIsEnabled(IsEnabled);
	TexturesDropdown->SetIsEnabled(IsEnabled);
	EffectsDropdown->SetIsEnabled(IsEnabled);
	FoliageDropdown->SetIsEnabled(IsEnabled);
	ShadingDropdown->SetIsEnabled(IsEnabled);
}

void USettingsMenuWidget::LoadCurrentFromSettings() const
{
	if(IsValid(WindowedModeToggle))
	{
		if(GameUserSettings->GetFullscreenMode() == EWindowMode::Fullscreen)
		{
			WindowedModeToggle->SetToggleState(false);
		}
		else WindowedModeToggle->SetToggleState(true);
	}

	if(IsValid(DisplayResolutionDropdown))
	{
		//Screen res
		const FIntPoint CurrentRes = GameUserSettings->GetScreenResolution();
		DisplayResolutionDropdown->SetSelectedOption(FString::FromInt(CurrentRes.X) + "x" + FString::FromInt(CurrentRes.Y));
	}

	if(IsValid(FramerateLimitDropdown))
	{
		const float FramerateLimit = GameUserSettings->GetFrameRateLimit();
		if(FramerateLimit == 0) FramerateLimitDropdown->SetSelectedOption("Unlimited");
		//we can always use from int as the rate should be a whole number
		else FramerateLimitDropdown->SetSelectedOption(FString::FromInt(FramerateLimit) + " FPS");
	}

	if(IsValid(ScalabilityPresetDropdown))
	{
		const int32 ScalabilityLevel = GameUserSettings->GetOverallScalabilityLevel();
		if(ScalabilityLevel >= 0)
		{
			ScalabilityPresetDropdown->SetSelectedIndex(ScalabilityLevel);
			LoadCurrentScalabilityFromSettings(ScalabilityLevel);
		}
		else
		{
			ScalabilityPresetDropdown->SetSelectedIndex(ScalabilityPresetDropdown->GetOptionCount() - 1); //The last option will be the "Manual" option
			LoadCurrentScalabilityFromSettings(ScalabilityPresetDropdown->GetSelectedIndex());
		}
	}

	//Get the player related settings and apply them (they will only be saved when triggering the Save Button)
	const APlayerPartyController* PartyController = Cast<APlayerPartyController>(GetWorld()->GetFirstPlayerController());
	if(IsValid(PartyController))
	{
		if(IsValid(PreferMostCenteredTargetToggle))
			PreferMostCenteredTargetToggle->SetToggleState(!PartyController->GetPlayerUserSettings().bPreferClosestTarget);
		if(IsValid(ZoomSpeedSlider))
			ZoomSpeedSlider->SetValue(PartyController->GetPlayerUserSettings().CameraZoomSpeed);
	}
	else UE_LOG(LogSaveGame, Warning, TEXT("Cannot load player settings as there seams to be no working player controller"));
}

void USettingsMenuWidget::LoadCurrentScalabilityFromSettings(int32 Scalability) const
{
	if(!(IsValid(ViewDistanceDropdown) && IsValid(AntiAliasingDistanceDropdown) && IsValid(PostProcessingDropdown) &&
		IsValid(ShadowsDropdown) && IsValid(GlobalIlluminationDropdown) && IsValid(ReflectionsDropdown) &&
		IsValid(TexturesDropdown) && IsValid(EffectsDropdown) && IsValid(FoliageDropdown) && IsValid(ShadingDropdown)))
	{
		UE_LOG(LogSaveGame, Warning, TEXT("Couldn't update scalability settings as one of the dropdowns is invalid."));
		return;
	}
	//5 or higher means we set the preset manually
	if(Scalability < ScalabilityPresetDropdown->GetOptionCount() - 1)
	{
		SetAllScalabilityIsEnabled(false);
		
		ViewDistanceDropdown->SetSelectedIndex(Scalability);
		AntiAliasingDistanceDropdown->SetSelectedIndex(Scalability);
		PostProcessingDropdown->SetSelectedIndex(Scalability);
		ShadowsDropdown->SetSelectedIndex(Scalability);
		GlobalIlluminationDropdown->SetSelectedIndex(Scalability);
		ReflectionsDropdown->SetSelectedIndex(Scalability);
		TexturesDropdown->SetSelectedIndex(Scalability);
		EffectsDropdown->SetSelectedIndex(Scalability);
		FoliageDropdown->SetSelectedIndex(Scalability);
		ShadingDropdown->SetSelectedIndex(Scalability);
	}
	else
	{
		SetAllScalabilityIsEnabled(true);
		
		ViewDistanceDropdown->SetSelectedIndex(GameUserSettings->GetViewDistanceQuality());
		AntiAliasingDistanceDropdown->SetSelectedIndex(GameUserSettings->GetAntiAliasingQuality());
		PostProcessingDropdown->SetSelectedIndex(GameUserSettings->GetPostProcessingQuality());
		ShadowsDropdown->SetSelectedIndex(GameUserSettings->GetShadowQuality());
		GlobalIlluminationDropdown->SetSelectedIndex(GameUserSettings->GetGlobalIlluminationQuality());
		ReflectionsDropdown->SetSelectedIndex(GameUserSettings->GetReflectionQuality());
		TexturesDropdown->SetSelectedIndex(GameUserSettings->GetTextureQuality());
		EffectsDropdown->SetSelectedIndex(GameUserSettings->GetVisualEffectQuality());
		FoliageDropdown->SetSelectedIndex(GameUserSettings->GetFoliageQuality());
		ShadingDropdown->SetSelectedIndex(GameUserSettings->GetShadowQuality());
	}
}

void USettingsMenuWidget::SetOptimalSettings()
{
	GameUserSettings->RunHardwareBenchmark();
	GameUserSettings->ApplyHardwareBenchmarkResults();
	//this implies there should be 5 or more settings (and "manual" should be the last one)
	ScalabilityPresetDropdown->SetSelectedIndex(ScalabilityPresetDropdown->GetOptionCount()-1);
	LoadCurrentFromSettings();
}

void USettingsMenuWidget::ApplySettings()
{
	//Get the state of all (standard UE5) settings
	if(WindowedModeToggle->GetToggleState()) GameUserSettings->SetFullscreenMode(EWindowMode::Windowed);
	else GameUserSettings->SetFullscreenMode(EWindowMode::Fullscreen);

	const FString DisplayRes = DisplayResolutionDropdown->GetSelectedOption();
	FString Left, Right;
	UKismetStringLibrary::Split(DisplayRes, "x", Left, Right);
	const FIntPoint NewRes(UKismetStringLibrary::Conv_StringToInt(Left), UKismetStringLibrary::Conv_StringToInt(Right));
	GameUserSettings->SetScreenResolution(NewRes);

	FString FramerateSetting = FramerateLimitDropdown->GetSelectedOption();
	if(FramerateSetting.RemoveFromEnd(" FPS"))
	{
		GameUserSettings->SetFrameRateLimit(UKismetStringLibrary::Conv_StringToInt(FramerateSetting));
	}
	else if(FramerateSetting == FString("Unlimited")){
		GameUserSettings->SetFrameRateLimit(0);
	}
	
	if(ScalabilityPresetDropdown->GetSelectedIndex() == ScalabilityPresetDropdown->GetOptionCount() - 1)
	{
		GameUserSettings->SetViewDistanceQuality(ViewDistanceDropdown->GetSelectedIndex());
		GameUserSettings->SetAntiAliasingQuality(AntiAliasingDistanceDropdown->GetSelectedIndex());
		GameUserSettings->SetPostProcessingQuality(PostProcessingDropdown->GetSelectedIndex());
		GameUserSettings->SetShadowQuality(ShadowsDropdown->GetSelectedIndex());
		GameUserSettings->SetGlobalIlluminationQuality(GlobalIlluminationDropdown->GetSelectedIndex());
		GameUserSettings->SetReflectionQuality(ReflectionsDropdown->GetSelectedIndex());
		GameUserSettings->SetTextureQuality(TexturesDropdown->GetSelectedIndex());
		GameUserSettings->SetVisualEffectQuality(EffectsDropdown->GetSelectedIndex());
		GameUserSettings->SetFoliageQuality(FoliageDropdown->GetSelectedIndex());
		GameUserSettings->SetShadingQuality(ShadingDropdown->GetSelectedIndex());
	}
	else
	{
		GameUserSettings->SetOverallScalabilityLevel(ScalabilityPresetDropdown->GetSelectedIndex());
	}

	//Apply the (Standard UE5) Settings
	GameUserSettings->ApplySettings(false);

	//Get the player related settings and apply them (they will only be saved when triggering the Save Button)
	APlayerPartyController* PartyController = Cast<APlayerPartyController>(GetWorld()->GetFirstPlayerController());
	if(IsValid(PartyController))
	{
		FPlayerUserSettings PlayerUserSettings;
		PlayerUserSettings.bPreferClosestTarget = !PreferMostCenteredTargetToggle->GetToggleState();
		PlayerUserSettings.CameraZoomSpeed = ZoomSpeedSlider->GetValue();
		PartyController->SetPlayerUserSettings(PlayerUserSettings, FSetPlayerUserSettingsKey());
	}
}

void USettingsMenuWidget::OnScalabilityPresetChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	LoadCurrentScalabilityFromSettings(ScalabilityPresetDropdown->GetSelectedIndex());
}

void USettingsMenuWidget::SaveGame()
{
	ACustomGameState* GameMode = GetWorld()->GetGameState<ACustomGameState>();
	if(IsValid(GameMode))
	{
		GameMode->WriteSaveGame();
	}
}
