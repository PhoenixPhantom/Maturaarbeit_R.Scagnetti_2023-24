// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetInformationComponent.h"


// Sets default values for this component's properties
UTargetInformationComponent::UTargetInformationComponent() : bCanBeTargeted(true),
	TargetPriority(100.f)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTargetInformationComponent::AddTargetingEntity(AController* TargetingController, FSetTargetStateKey)
{
	TargetingControllers.AddUnique(TargetingController);
}

void UTargetInformationComponent::RemoveTargetingEntity(AController* NonTargetingController, FSetTargetStateKey)
{
	TargetingControllers.RemoveSwap(NonTargetingController);
}

