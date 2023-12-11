// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PatrolPath.generated.h"

UCLASS()
class MAPROJECT_API APatrolPath : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APatrolPath();
	
	FORCEINLINE FVector GetAbsolutePointLocation(int32 Index) const;
	FORCEINLINE int32 GetNumPathPoints() const { return PathPoints.Num(); }
	FORCEINLINE void GetPathPoints(TArray<FVector>& OutPathPoints) const { OutPathPoints = PathPoints; }
	FORCEINLINE bool IsLoopingPathPoints() const { return bLoopPathPoints; }
	
protected:
	UPROPERTY(EditAnywhere, meta=(MakeEditWidget))
	TArray<FVector> PathPoints;

	//Indicates if the path forms a loop (e.g. we go from IndexMax to 0, 1, 3, etc ...
	//instead of back to IndexMax-1, IndexMax-2, etc ...) 
	UPROPERTY(EditAnywhere)
	bool bLoopPathPoints;
};
