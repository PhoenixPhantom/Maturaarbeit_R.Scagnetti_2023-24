// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "Characters/Fighters/FighterCharacter.h"
#include "BTTask_CustomMoveTo.generated.h"

UENUM()
enum class EForcedMovementType : uint8
{
	PreferCurrent,
	ForceKeepCurrent,
	PreferWalking,
	PreferRunning,
	ForceWalking,
	ForceRunning
};

/**
 * 
 */
UCLASS()
class MAPROJECT_API UBTTask_CustomMoveTo : public UBTTask_MoveTo
{
	GENERATED_BODY()
public:
	UBTTask_CustomMoveTo();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere)
	EForcedMovementType ForcedMovementType;

	EBTNodeResult::Type ExecutePreferCurrent(AFighterCharacter* Character, UBehaviorTreeComponent& OwnerComp,
													  uint8* NodeMemory);
	EBTNodeResult::Type ExecuteForceKeepCurrent(AFighterCharacter* Character, UBehaviorTreeComponent& OwnerComp,
													  uint8* NodeMemory);
	EBTNodeResult::Type ExecutePreferWalking(AFighterCharacter* Character, UBehaviorTreeComponent& OwnerComp,
													  uint8* NodeMemory);
	EBTNodeResult::Type ExecutePreferRunning(AFighterCharacter* Character, UBehaviorTreeComponent& OwnerComp,
													  uint8* NodeMemory);

	EBTNodeResult::Type ExecuteForceWalking(AFighterCharacter* Character, UBehaviorTreeComponent& OwnerComp,
													  uint8* NodeMemory);
	EBTNodeResult::Type ExecuteForceRunning(AFighterCharacter* Character, UBehaviorTreeComponent& OwnerComp,
													  uint8* NodeMemory);
	
};
