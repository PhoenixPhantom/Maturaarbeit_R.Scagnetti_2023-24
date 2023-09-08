// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/Navigation/PatrolPath.h"

#include "Kismet/KismetSystemLibrary.h"


// Sets default values
APatrolPath::APatrolPath() : bLoopPathPoints(false)
{
	PrimaryActorTick.bCanEverTick = false;
}
