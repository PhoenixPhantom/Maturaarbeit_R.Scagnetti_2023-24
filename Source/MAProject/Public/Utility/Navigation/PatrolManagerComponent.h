// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PatrolManagerComponent.generated.h"


class APatrolPath;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MAPROJECT_API UPatrolManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPatrolManagerComponent();

	FORCEINLINE const APatrolPath* GetPatrolPath() const { return PatrolPath; }
	FVector GetNextPathPointLocation(bool StartFromClosest = false);
	int32 AdvanceToNextPathPoint();

protected:
	UPROPERTY(SaveGame)
	int32 NextIndex;

	//Whether the next index will be lower or higher than the current one
	UPROPERTY(SaveGame)
	bool bIncreasingIndex;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=BaseSettings)
	APatrolPath* PatrolPath;

	void GetPreferredNextPoint();
};
