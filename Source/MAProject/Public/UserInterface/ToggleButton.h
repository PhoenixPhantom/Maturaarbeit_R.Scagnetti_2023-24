// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "ToggleButton.generated.h"

/**
 * 
 */
UCLASS()
class MAPROJECT_API UToggleButton : public UButton
{
	GENERATED_BODY()
public:
	UToggleButton(const FObjectInitializer& ObjectInitializer);

	void SetToggleState(bool IsOn);
	bool GetToggleState() const { return bIsOn; }
	
protected:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Apparence", meta=(Displayname="ToggleState"))
	bool bIsOn;

	UPROPERTY(EditAnywhere, Category="Appearance", meta=( DisplayName="Style" ))
	FButtonStyle ToggleOther;
	
	UFUNCTION()
	void OnButtonClicked();
};
