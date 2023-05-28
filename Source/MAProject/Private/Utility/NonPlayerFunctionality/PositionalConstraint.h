// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PositionalConstraint.generated.h"


USTRUCT()
struct MAPROJECT_API FPositionalConstraint
{
	GENERATED_BODY()
public:
	FPositionalConstraint() = default;
	virtual ~FPositionalConstraint() = default;
	
	virtual bool IsConstraintSatisfied(FVector Position) { unimplemented(); return false; };
};
