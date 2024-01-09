// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "Utility/CombatManager.h"
#include "OpponentController.generated.h"

class AOpponentCharacter;
class UAISenseConfig_Sight;
class AMovementTarget;
class ACombatManager;
class UCrowdFollowingComponent;

struct FForceEndCombatKey final
{
	friend class ACombatManager;
private:
	FForceEndCombatKey(){}
};

struct FReleaseTokenKey final
{
	friend class UBTTask_ReleaseAggressionTokens;
private:
	FReleaseTokenKey(){}
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


struct FTimestampedStimulus : public FAIStimulus
{
	FTimestampedStimulus() : Timestamp(0.f), TargetActor(nullptr)
	{}

	FTimestampedStimulus(double CurrentTime) : Timestamp(CurrentTime), TargetActor(nullptr){}

	FTimestampedStimulus(const FAIStimulus& NewStimulus, double CurrentTime, AActor* Target);
	double Timestamp;
	AActor* TargetActor;

	bool operator==(const FTimestampedStimulus& Comp) const;
	FORCEINLINE bool operator!=(const FTimestampedStimulus& Comp) const{ return !operator==(Comp); }
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
	virtual void Tick(float DeltaSeconds) override;

	
	virtual FGenericTeamId GetGenericTeamId() const override { return 1; }

	void ForceEndCombat(bool FullyUnregister, FForceEndCombatKey){ EndCombat(FullyUnregister); }
	bool UpdateCombatLocation(FVector& ResultingLocation, ECombatParticipantStatus ParticipantStatus,
		bool ForceRecalculation = false) const;

	//We override the built in MoveTo function to make all move to requests use the custom MoveTarget so we can
	//have a smooth interpolation when movement targets are changed on the fly instead of always stopping and then
	//starting to walk every time we change the MoveTo target
	virtual FPathFollowingRequestResult MoveTo(const FAIMoveRequest& MoveRequest, FNavPathSharedPtr* OutPath = nullptr) override;
	
	float GetFieldOfView() const;
	ACombatManager* GetCombatManager() const{ return CombatManager; }

#if WITH_EDITORONLY_DATA
	bool GetIsDebugging() const { return bIsDebugging; }
	void SetIsDebugging(bool IsDebugging){ bIsDebugging = IsDebugging; }
#endif
	
protected:
	FGenericTeamId InternalTeamId;
	TArray<FTimestampedStimulus> LastSightStimuli;
	TArray<FTimestampedStimulus> TooCloseToForgetStimuli;
	
	UPROPERTY()
	AMovementTarget* MoveTarget;
	UPROPERTY()
	ACombatManager* CombatManager;
	UPROPERTY()
	AOpponentCharacter* ControlledOpponent;
	UPROPERTY()
	UCrowdFollowingComponent* CrowdFollowingComponent;

	UPROPERTY()
	AActor* ActorToInvestigate;

	UPROPERTY(EditAnywhere, Category = Blackboard)
	FName TargetLocationKeyName;
	UPROPERTY(EditAnywhere, Category = Blackboard)
	FName RestartPatrolPathKeyName;
	UPROPERTY(EditAnywhere, Category = Blackboard)
	FName IsInCombatKeyName;
	UPROPERTY(EditAnywhere, Category = Blackboard)
	FName IsActiveCombatKeyName;
	UPROPERTY(EditAnywhere, Category = Blackboard)
	FName IsInvestigatingKeyName;
	UPROPERTY(EditAnywhere, Category = Blackboard)
	FName HasJustExecutedAttackKeyName;
	UPROPERTY(EditAnywhere, Category = Blackboard)
	FName LastCombatStatusKeyName;

	UPROPERTY(EditAnywhere, Category = Movement)
	TSubclassOf<AMovementTarget> MovementTargetClass;

	UPROPERTY(EditAnywhere, Category = Perception)
	float RelevantSightPerceptionChangeRadius;
	
	UPROPERTY(EditAnywhere, Category = Combat, AdvancedDisplay)
	float ForwardSampleNumber;
	
	UPROPERTY(EditAnywhere, Category = General)
	UBehaviorTree* DefaultBehaviorTree;

	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

	void TriggerInvestigationProcess(const FAIStimulus& KnownInformation) const;

	//calling this implies that KnownInformation.Type == Sight
	void ActiveUpdateCombat(const AActor* CombatTarget, const FAIStimulus& KnownInformation) const;
	void EndCombat(bool FullyUnregister = false) const;

	bool OnSightForgotten(AActor* SightedActor) const;

	static FVector GetCharacterTargetLocation(const AOpponentCharacter* RelevantCharacter, FName BlackboardTargetLocationName);

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* UpdatedActor, FAIStimulus Stimulus);

	UFUNCTION()
	void OnAggressionTokenGranted();

	UFUNCTION()
	void OnAggressionTokenReleased();

	UFUNCTION()
	void OnFlickBackTriggered(FAIRequestID RequestID, EPathFollowingResult::Type Result);


#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = Debugging)
	bool bIsDebugging = false;
	UFUNCTION(CallInEditor, Category = Debugging)
	void ToggleDebugging();
#endif	
};
