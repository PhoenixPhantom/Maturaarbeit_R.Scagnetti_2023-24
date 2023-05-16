// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "OpponentController.generated.h"

class UAISenseConfig_Sight;
class AMovementTarget;

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
	
	FTimerHandle LostPerceptionHandle;	

	UPROPERTY()
	AActor* CurrentTarget;
	UPROPERTY()
	AMovementTarget* MoveTarget;

	UPROPERTY(EditAnywhere)
	UBehaviorTree* DefaultBehaviorTree;
	
	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* UpdatedActor, FAIStimulus Stimulus);

#if WITH_EDITORONLY_DATA
	UFUNCTION(CallInEditor)
	void ToggleDebugging() const;
#endif	
};
