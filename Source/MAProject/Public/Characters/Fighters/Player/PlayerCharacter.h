// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Fighters/FighterCharacter.h"
#include "InputActionValue.h"
#include "PlayerPartyController.h"
#include "PlayerCharacter.generated.h"

class UPlayerStatsMonitorBaseWidget;
class USphereComponent;
class UTargetInformationComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UStatsMonitorBaseWidget;
class ACombatManager;

DECLARE_DELEGATE(FOnInputRepeatedDelegate);

struct FPreSpawnSetupKey final
{
	friend class APlayerPartyController;
private:
	FPreSpawnSetupKey(){}
};

struct FSetIsRestoringHealthKey final
{
	friend class ACombatManager;
private:
	FSetIsRestoringHealthKey(){}
};

struct FStoredInput
{
	double Timestamp;
	EInputType ActionType;
	TDelegate<void()> RequestedAction;

	FStoredInput();
	FStoredInput(double CurrentTime, EInputType AttemptedActionType, const TDelegate<void()>& AttemptedAction);
	void TryUpdateStoredInput(double CurrentTime, double MaxInputWindow, EInputType AttemptedActionType, const TFunction<void()>& AttemptedAction);
	bool IsValid() const { return ActionType != EInputType::Undefined && RequestedAction.IsBound(); }
	void Invalidate();

	bool operator==(const FStoredInput& SavedInput) const;
};


UCLASS()
class MAPROJECT_API APlayerCharacter : public AFighterCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter(const FObjectInitializer& ObjectInitializer);
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;

	virtual float GetFieldOfView() const override;

	virtual FGenericTeamId GetGenericTeamId() const override { return InternalTeamId; }
	void SetIsRestoringHealth(bool ShouldRestore, FSetIsRestoringHealthKey){ bIsRestoringHealth = ShouldRestore; }
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	const FCharacterBaseStats& GetCharacterBaseStats() const { return BaseStats; }
	void PreSpawnSetup(FCharacterStats* PropertiesSource, FPlayerUserSettings* PlayerUserSettingsSource,
		const TDelegate<void(const FVector2D&)>& RequestedActionOnPlayerMovedCamera, FGenericTeamId NewTeamId,
		FPreSpawnSetupKey Key);


	/**
	 * @brief Calculate the action rank the given target has in relation to the player. This takes screen centered-ness
	 * as well as whether the target is on-screen 
	 * @param RankGenerationTarget the target for which we calculate the action rank 
	 * @return the score calculated >= 0.f*/
	float RequestActionRank(const AActor* RankGenerationTarget) const;
	
	///@return CameraBoom sub-object
	FORCEINLINE USpringArmComponent* GetSpringArm() const { return SpringArm; }
	///@return FollowCamera sub-object
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	
	UTargetInformationComponent* GetCurrentTargetInformation() const { return CurrentTarget; }
	AActor* GetCurrentTarget() const;

protected:
	uint8 bIsRunning:1;
	uint8 bHasJumped:1;
	uint8 bIsRestoringHealth:1;
	double RestoreHealthTimestamp;
	double DashOrBlinkTimestamp;

	FGenericTeamId InternalTeamId;

	FTimerHandle ResetPlayerVisibilityHandle;
	TTuple<double, FVector> InputDirection;
	FPlayerUserSettings* PlayerUserSettings;
	FStoredInput LastInput;
	TDelegate<void(const FVector2D&)> OnPlayerMovedCamera;
	TDelegate<void(bool)> OnAttackInterrupted;

	

	UPROPERTY()
	UTargetInformationComponent* CurrentTarget;

	UPROPERTY()
	UPlayerStatsMonitorBaseWidget* PlayerStatsMonitor;
	
	UPROPERTY(EditAnywhere, Category = Combat, AdvancedDisplay)
	float AutotargetingRange;

	UPROPERTY(EditAnywhere, Category = Input, AdvancedDisplay)
	double DashOrBlinkCooldown;
	
	UPROPERTY(EditAnywhere, Category = Input, AdvancedDisplay)
	double RememberInputDirectionTime;

	UPROPERTY(EditAnywhere, Category = Input, AdvancedDisplay)
	double MaximalInputWindowTime;

	UPROPERTY(EditAnywhere)
	UCapsuleComponent* HorizontalCapsule;

	UPROPERTY(EditAnywhere, Category = UserExperience)
	TSubclassOf<UCameraShakeBase> InduceStaggerCameraShake;

	UPROPERTY(EditAnywhere, Category = UserInterface)
	TSubclassOf<UPlayerStatsMonitorBaseWidget> HealthWidgetClass;
	
	UPROPERTY(EditAnywhere, Category = UserInterface)
	TSubclassOf<UUserWidget> PauseMenuClass;

	UPROPERTY(EditAnywhere, Category = UserInterface)
	TSubclassOf<UUserWidget> DeathMenuClass;
	
	//MappingContext
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, AdvancedDisplay)
	UInputAction* JumpAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, AdvancedDisplay)
    UInputAction* LightAttackAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, AdvancedDisplay)
	UInputAction* HeavyAttackAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, AdvancedDisplay)
	UInputAction* SkillAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, AdvancedDisplay)
	UInputAction* UltimateAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, AdvancedDisplay)
	UInputAction* DashAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, AdvancedDisplay)
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, AdvancedDisplay)
	UInputAction* LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, AdvancedDisplay)
	UInputAction* CameraZoomAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, AdvancedDisplay)
	UInputAction* AimAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, AdvancedDisplay)
	UInputAction* PauseMenuAction;

	//Camera boom positioning the camera behind the character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* SpringArm;

	//Follow camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual bool TriggerToughnessBroken() override;

	virtual float GetLegIKBlendWeight(const FVector& Velocity) override;
	virtual void QueueFollowUpLimit(const TArray<FNewInputLimits>& InputLimits) override;
	virtual void GenerateDamageEvent(FAttackDamageEvent& AttackDamageEvent, const FHitResult& CausingHit) override;
	virtual bool TriggerDeath() override;
	virtual void OnHitTimeDilation(bool WasStaggered) override{} // the player should not get staggered on enemy attacks

	virtual void CharacterLanded() override;
	virtual void CharacterInAir() override;

	virtual void OnAttackTreeModeChanged(FString NewRoot) override;

	virtual void OnNewStatusEffectReceived(UStatusEffect* StatusEffect) override;
	virtual void OnStatusEffectRemoved() override;

	
	void TryJump();
	void EndJump();
	
	void LightAttack();
	void HeavyAttack();
	void SkillAttack();
	void UltimateAttack();

	
	///Execute blink @return whether the blink could be fully executed
	bool Blink();
	void DashStartRunning();
	void StopRunning();
	void Move(const FInputActionValue& Value);
	
	void Look(const FInputActionValue& Value);
	void CameraZoom(const FInputActionValue& Value);
	void Aim(const FInputActionValue& Value);

	void OpenPauseMenu();

	void UpdateTargetSelection();
	bool IsOccluded(ETraceTypeQuery TraceType, const FVector& ObserverLocation, const FVector& TargetCenter, const FVector& TargetExtent, AActor* TargetActor) const;

	UFUNCTION()
	void ShowDeathMenu(bool IsLimitDurationOver);
	UFUNCTION()
	void OnCdSet(UAttackNode* IdentifiedNode, int32 Index);
	UFUNCTION()
	void AttackInterrupted(bool IsLimitDurationOver);
	UFUNCTION()
	void OnSelectMotionWarpingTarget(const FAttackProperties& Properties);
};