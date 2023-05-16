// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "OpponentController.generated.h"

class UAISenseConfig_Sight;
class AMovementTarget;

DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(float, FOnGenerateAggressionScoreDelegate, AOpponentCharacter*, TargetCharacter);

/**
 * 
 */
UCLASS()
class MAPROJECT_API AOpponentController : public AAIController
{
	GENERATED_BODY()
public:
	AOpponentController();
	
	float GetFieldOfView() const;
	
	virtual FPathFollowingRequestResult MoveTo(const FAIMoveRequest& MoveRequest, FNavPathSharedPtr* OutPath) override;
protected:
	uint8 bIsAggressive:1;
	uint8 bCanBecomeAggressive:1;
	uint8 bIsCurrentTarget:1;

	FOnGenerateAggressionScoreDelegate OnGenerateAggressionScore;
	
	FTimerHandle LostPerceptionHandle;	

	UPROPERTY()
	AActor* CurrentTarget;
	UPROPERTY()
	AMovementTarget* MoveTarget;

	UPROPERTY()
	AOpponentCharacter* OpponentCharacter;

	UPROPERTY(EditAnywhere)
	UBehaviorTree* DefaultBehaviorTree;
	
	virtual void OnPossess(APawn* InPawn) override;

	float GenerateAggressionScore();

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* UpdatedActor, FAIStimulus Stimulus);

	UFUNCTION()
	void OnTargetStateChanged(bool IsCurrentTarget);

#if WITH_EDITORONLY_DATA
	UFUNCTION(CallInEditor)
	void ToggleDebugging() const;
#endif	
};
