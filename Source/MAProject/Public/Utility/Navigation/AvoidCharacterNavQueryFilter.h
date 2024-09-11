// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NavFilters/NavigationQueryFilter.h"
#include "AvoidCharacterNavQueryFilter.generated.h"

/**
 * 
 */
UCLASS()
class MAPROJECT_API UAvoidCharacterNavQueryFilter : public UNavigationQueryFilter
{
	GENERATED_BODY()
public:
	UAvoidCharacterNavQueryFilter();
};
