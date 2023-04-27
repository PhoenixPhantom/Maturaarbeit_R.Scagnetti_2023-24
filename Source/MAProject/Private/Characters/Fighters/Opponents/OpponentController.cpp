// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/OpponentController.h"

#include "BehaviorTree/BehaviorTree.h"

AOpponentController::AOpponentController() : DefaultBehaviorTree(nullptr)
{
}

void AOpponentController::OnPossess(APawn* InPawn)
{
	RunBehaviorTree(DefaultBehaviorTree);
	Super::OnPossess(InPawn);
}
