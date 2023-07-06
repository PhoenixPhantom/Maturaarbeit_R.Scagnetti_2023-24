// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <functional>

#include "CoreMinimal.h"
#include "Characters/Fighters/FighterCharacter.h"
#include "InputActionValue.h"
#include "PlayerPartyController.h"
#include "PlayerCharacter.generated.h"

class USphereComponent;
class UTargetInformationComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;

template<typename F, typename S, typename T>
struct TTriple
{
	TTriple() : bIsValid(false){}
	TTriple(F NewFirst, S NewSecond, T NewThird) : bIsValid(true), First(NewFirst), Second(NewSecond), Third(NewThird){}
	bool bIsValid;
	F First;
	S Second;
	T Third;

	template<typename F2, typename S2, typename T2>
	TTriple operator=(const TTriple<F2, S2, T2>& Other){ bIsValid = Other.bIsValid; First = Other.First;
		Second = Other.Second; Third = Other.Third; return *this; }
};

DECLARE_DELEGATE(FOnInputRepeatedDelegate);

struct FPreSpawnSetupKey final
{
	friend class APlayerPartyController;
private:
	FPreSpawnSetupKey(){}
};

UCLASS()
class MAPROJECT_API APlayerCharacter : public AFighterCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;

	virtual float GetFieldOfView() const override;

	const FCharacterBaseStats& GetCharacterBaseStats() const { return BaseStats; }
	void PreSpawnSetup(FCharacterStats* PropertiesSource, FPlayerUserSettings* PlayerUserSettingsSource, FPreSpawnSetupKey Key);


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


protected:
	bool bIsRunning;
	bool bHasJumped;

	FTimerHandle ResetPlayerVisibilityHandle;
	TTuple<double, FVector> InputDirection;
	FPlayerUserSettings* PlayerUserSettings;
	TTriple<double, EInputType, std::function<void()>> LastInput;

	

	UPROPERTY()
	UTargetInformationComponent* CurrentTarget;

	UPROPERTY(EditAnywhere, Category = Combat, AdvancedDisplay)
	float AutotargetingRange;

	UPROPERTY(EditAnywhere, Category = Input, AdvancedDisplay)
	double RememberInputDirectionTime;

	UPROPERTY(EditAnywhere, Category = Input, AdvancedDisplay)
	double RememberLastInputTime;
	
	UPROPERTY(EditAnywhere, Category = UserInterface)
	TSubclassOf<UUserWidget> PauseMenuClass;
	
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

	virtual void QueueFollowUpLimit(const TArray<FInputLimits>& InputLimits, int32 CurrentLimitIndex) override;
	
	void TryJump();
	void EndJump();
	
	void LightAttack();
	void HeavyAttack();
	void SkillAttack();
	void UltimateAttack();

	void SpeedUpDash();
	void SlowDown();
	void Move(const FInputActionValue& Value);
	
	void Look(const FInputActionValue& Value);
	void CameraZoom(const FInputActionValue& Value);
	void Aim(const FInputActionValue& Value);

	void OpenPauseMenu();

	void UpdateTargetSelection();
	
	UFUNCTION()
	void OnSelectMotionWarpingTarget(const FAttackProperties& Properties);

	enum EAttackType
	{
		Light,
		Heavy,
		Skill,
		Ultimate
	};
};