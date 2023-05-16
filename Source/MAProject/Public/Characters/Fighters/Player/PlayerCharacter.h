// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Fighters/FighterCharacter.h"
#include "InputActionValue.h"
#include "PlayerPartyController.h"
#include "../../../../../../../../../../../Program Files/Epic Games/UE_5.1/Engine/Platforms/Hololens/Source/Runtime/Core/Public/Microsoft/AllowMicrosoftPlatformTypesPrivate.h"
#include "PlayerCharacter.generated.h"

class USphereComponent;
class UTargetInformationComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;


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
	
	///@return CameraBoom sub-object
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	///@return FollowCamera sub-object
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	
	UFUNCTION()
	float OnGenerateAggressionScore(AOpponentCharacter* TargetCharacter);

protected:
	bool bIsRunning;
	bool bHasJumped;

	FTimerHandle ResetPlayerVisibilityHandle;
	TTuple<double, FVector> InputDirection;
	FPlayerUserSettings* PlayerUserSettings;

	UPROPERTY()
	UTargetInformationComponent* CurrentTarget;

	UPROPERTY(EditAnywhere, Category = Combat, AdvancedDisplay)
	float AutotargetingRange;

	UPROPERTY(EditAnywhere, Category = Input, AdvancedDisplay)
	double RememberInputDirectionTime = 0.5;
	
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
	USpringArmComponent* CameraBoom;

	//Follow camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

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