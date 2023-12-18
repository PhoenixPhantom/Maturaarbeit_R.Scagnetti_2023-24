// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/Navigation/PatrolPath.h"

#include "Kismet/KismetSystemLibrary.h"


// Sets default values
APatrolPath::APatrolPath() : bLoopPathPoints(false)
{
	PrimaryActorTick.bCanEverTick = false;
}

FVector APatrolPath::GetAbsolutePointLocation(int32 Index) const
{
	if(!PathPoints.IsValidIndex(Index))
	{
		return FVector(NAN);
	}
	return GetActorLocation() + PathPoints[Index];
}
