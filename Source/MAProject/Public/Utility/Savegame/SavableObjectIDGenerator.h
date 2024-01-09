// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SavableObjectIDGenerator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUniqueWorldId, Error, All);

uint64 GetLowestValidID(const TArray<uint64>& IDsInUse);

UCLASS()
class MAPROJECT_API ASavableObjectIDGenerator : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASavableObjectIDGenerator();
	virtual bool IsEditorOnly() const override;

protected:
	//Turning off this functionality means, that a once populated ID will not be restored until all other indices were used
	//this means that when first shipping the project this option should be enabled to not invalidate existing players
	//save data (by loading data from a completely unrelated saved object to a new one (that took over the old ones index))
	UPROPERTY(EditAnywhere, Category = "ID Generation")
	bool bAllowReusingIndices;

	virtual void BeginPlay() override;

	UFUNCTION(CallInEditor, Category = "ID Generation")
	void Recalculate() const;
};
