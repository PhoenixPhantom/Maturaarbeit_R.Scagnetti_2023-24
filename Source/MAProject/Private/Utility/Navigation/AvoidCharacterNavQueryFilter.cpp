// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/Navigation/AvoidCharacterNavQueryFilter.h"
#include "Utility/Navigation/CharacterNavigationArea.h"

UAvoidCharacterNavQueryFilter::UAvoidCharacterNavQueryFilter()
{
	FNavigationFilterArea CharacterFilterArea;
	CharacterFilterArea.AreaClass = UCharacterNavigationArea::StaticClass();
	//CharacterFilterArea.bIsExcluded = true;
	CharacterFilterArea.bOverrideTravelCost = true;
	CharacterFilterArea.bOverrideEnteringCost = true;
	CharacterFilterArea.EnteringCostOverride = FLT_MAX;
	CharacterFilterArea.TravelCostOverride = FLT_MAX;
	Areas.Add(CharacterFilterArea);
}
