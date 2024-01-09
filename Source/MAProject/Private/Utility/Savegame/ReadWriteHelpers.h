// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
namespace UReadWriteHelpers
{
	void WriteToTarget(AActor* Target, TArray<uint8>& Bytes);
	void ReadFromTarget(AActor* Target, TArray<uint8>& Bytes);
}
