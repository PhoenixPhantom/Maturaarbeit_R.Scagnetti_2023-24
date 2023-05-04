// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetInformationComponent.h"


// Sets default values for this component's properties
UTargetInformationComponent::UTargetInformationComponent() : TargetPriority(100.f), bIsCurrentTarget(false)
{
	PrimaryComponentTick.bCanEverTick = false;
}

