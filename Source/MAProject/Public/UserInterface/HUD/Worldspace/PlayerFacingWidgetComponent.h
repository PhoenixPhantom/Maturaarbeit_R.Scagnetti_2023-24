// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "PlayerFacingWidgetComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthMonitorWidgetInitializedDelegate, UStatsMonitorBaseWidget*, Widget);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MAPROJECT_API UPlayerFacingWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	
	FOnHealthMonitorWidgetInitializedDelegate OnHealthMonitorWidgetInitialized;
	
	// Sets default values for this component's properties
	UPlayerFacingWidgetComponent();
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
};
