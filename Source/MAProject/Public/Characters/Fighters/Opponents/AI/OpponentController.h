// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "OpponentController.generated.h"

class AOpponentCharacter;
class UAISenseConfig_Sight;
class AMovementTarget;
class ACombatManager;
class UCrowdFollowingComponent;

struct FReleaseTokenKey final
{
	friend class UBTTask_ReleaseAggressionTokens;
private:
	FReleaseTokenKey(){};
};

//We need this child class so we can easily change the GoalActor/Location of the AIMoveRequest (which is not easily
//possible in the parent class)
USTRUCT()
struct FAIMoveRequestExpanded : public FAIMoveRequest
{
	GENERATED_BODY()
public:
	FAIMoveRequestExpanded() = default;
	FAIMoveRequestExpanded(const FAIMoveRequest& Request) : FAIMoveRequest(Request){}
	
	void ForceSetGoalActor(const AActor* InGoalActor);
	void ForceSetGoalLocation(const FVector& InGoalLocation);
};

/**
 * 
 */
UCLASS()
class MAPROJECT_API AOpponentController : public AAIController
{
	GENERATED_BODY()
public:
	AOpponentController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	bool GenerateTargetLocation(FVector& OptimalLocation) const;
	bool IsValidTargetLocation(const FVector& TargetLocation) const;

	//We override the built in MoveTo function to make all move to requests use the custom MoveTarget so we can
	//have a smooth interpolation when movement targets are changed on the fly instead of always stopping and then
	//starting to walk every time we change the MoveTo target
	virtual FPathFollowingRequestResult MoveTo(const FAIMoveRequest& MoveRequest, FNavPathSharedPtr* OutPath = nullptr) override;
	
	float GetFieldOfView() const;
	ACombatManager* GetCombatManager() const{ return CombatManager; }
	
protected:
	FTimerHandle LostPerceptionHandle;	
	
	UPROPERTY()
	AMovementTarget* MoveTarget;
	UPROPERTY()
	ACombatManager* CombatManager;
	UPROPERTY()
	AOpponentCharacter* ControlledOpponent;
	UPROPERTY()
	UCrowdFollowingComponent* CrowdFollowingComponent;
	
	UPROPERTY(EditAnywhere)
	float ForwardSampleNumber;
	
	UPROPERTY(EditAnywhere)
	UBehaviorTree* DefaultBehaviorTree;

	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

	void RegisterSensedPlayer(AController* Player);
	void UnregisterSensedPlayer(AController* Player);

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* UpdatedActor, FAIStimulus Stimulus);

	UFUNCTION()
	void OnAggressionTokenGranted();

	UFUNCTION()
	void OnAggressionTokenReleased();

	UFUNCTION()
	void OnFlickBackTriggered(FAIRequestID RequestID, EPathFollowingResult::Type Result);


#if WITH_EDITORONLY_DATA
	bool bIsDebugging = false;
	UFUNCTION(CallInEditor)
	void ToggleDebugging();
#endif	
};
