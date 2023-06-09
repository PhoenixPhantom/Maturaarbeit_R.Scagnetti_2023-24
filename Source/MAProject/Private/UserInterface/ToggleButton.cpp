// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/ToggleButton.h"

UToggleButton::UToggleButton(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	OnClicked.AddDynamic(this, &UToggleButton::OnButtonClicked);
}

void UToggleButton::SetToggleState(bool IsOn)
{
	//We only have to do something if we are in the wrong state
	if(bIsOn != IsOn) OnButtonClicked();
	
}

void UToggleButton::OnButtonClicked()
{
	const FButtonStyle TargetStyle = ToggleOther;
	bIsOn = !bIsOn;

	//make the other toggle be the current toggle
	ToggleOther = GetStyle();
	SetStyle(TargetStyle);
}