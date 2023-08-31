// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/HUD/Worldspace/PlayerFacingWidgetComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "UserInterface/HealthMonitorBaseWidget.h"


// Sets default values for this component's properties
UPlayerFacingWidgetComponent::UPlayerFacingWidgetComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UPlayerFacingWidgetComponent::BeginPlay()
{
	Super::BeginPlay();
	UHealthMonitorBaseWidget* HealthMonitorWidget = CastChecked<UHealthMonitorBaseWidget>(GetWidget());
	OnHealthMonitorWidgetInitialized.Broadcast(HealthMonitorWidget);
}


// Called every frame
void UPlayerFacingWidgetComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                 FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if(HasBegunPlay() && GetWidget()->IsVisible())
	{
		SetWorldRotation(UKismetMathLibrary::FindLookAtRotation(GetComponentLocation(),
			GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetCameraLocation()));
	}
}

