// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputManagement.h"
#include "GameFramework/Character.h"
#include "GeneralCharacter.generated.h"

class UCustomAnimInstance;
class UStatusEffect;
class USuckToTargetComponent;

struct FSetCharacterOpacity final
{
	friend class AGeneralCharacter;
	friend class UAnimNotifyState_BlendVisibility;
	friend class UAnimNotify_InEditorResetVisibility;
	friend class UAnimNotifyState_ForceConstantVisibility;
private:
	FSetCharacterOpacity(){};
};

struct FModifyCharacterStatusEffectKey final
{
	friend class UAnimNotify_EnterStatusEffect;
	friend class UStatusEffect;
private:
	FModifyCharacterStatusEffectKey(){}
};

UCLASS(meta=(PrioritizeCategories = "Debugging Combat OpponentCharacter"))
class AGeneralCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGeneralCharacter(const FObjectInitializer& ObjectInitializer);

	
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
	virtual void GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;
	
	virtual float GetFieldOfView() const { unimplemented(); return 0.f; }
	
	/**
	 * @brief Get weather an input of the given can override the currently active one or not
	 * @param Type the input to check
	 * @return Weather the current input can be overridden by Type
	 */
	bool CanOverrideCurrentInput(EInputType Type) const { return AcceptedInputs.IsAllowedInput(Type); }
	const FAcceptedInputs& GetAcceptedInputs() const { return AcceptedInputs; }

	USuckToTargetComponent* GetSuckToTargetComponent() const{ return SuckToTargetComponent; }
	float GetMeshesOpacity() const;
	void SetMeshesOpacity(float DesiredOpacity, FSetCharacterOpacity);
	void SetAllowAutomaticOpacityChanges(bool ShouldAllow, FSetCharacterOpacity){ bAllowAutomaticOpacityChanges = ShouldAllow; };
	bool GetAllowAutomaticOpacityChanges() const { return bAllowAutomaticOpacityChanges; }

	UCustomAnimInstance* GetCustomAnimInstance() const { return CustomAnimInstance; };

	void ReceiveStatusEffectExternal(TSubclassOf<UStatusEffect> NewEffectType, FModifyCharacterStatusEffectKey){ ReceiveStatusEffect(NewEffectType); }
	void RemoveStatusEffectExternal(UStatusEffect* StatusEffect, FModifyCharacterStatusEffectKey){ RemoveStatusEffectInternal(StatusEffect); }

#if WITH_EDITORONLY_DATA
	void SetIsDebugging(bool IsDebugging);
	bool GetIsDebugging() const { return bIsDebugging; }
#endif

protected:
	FAcceptedInputs AcceptedInputs;
	bool bAllowAutomaticOpacityChanges;
	
	UPROPERTY()
	TArray<USkeletalMeshComponent*> RelevantMeshes;

	UPROPERTY(EditAnywhere, Category = Rendering)
	float MinimumFadeDistance;
	UPROPERTY(EditAnywhere, Category = Rendering)
	float MaximumFadeDistance;
	UPROPERTY(EditAnywhere, Category = Rendering)
	float InputFadeStrength;
	
	//The prefix (if existent) every bone on the characters skeleton has
	UPROPERTY(EditAnywhere, Category = Animation)
	FString BonePrefix;
	
	UPROPERTY(BlueprintReadWrite, Category = Animation)
	UCustomAnimInstance* CustomAnimInstance;

	UPROPERTY(EditAnywhere, Category = Animation)
	USuckToTargetComponent* SuckToTargetComponent;


	virtual void BeginPlay() override;

	virtual float GetLegIKBlendWeight(const FVector& Velocity);
	virtual void FadeMeshWithCameraDistance();
	virtual void CharacterLanded();
	virtual void CharacterInAir();
	
	virtual bool TriggerDeath();

	virtual void OnNewStatusEffectReceived(UStatusEffect* StatusEffect){}
	virtual void OnStatusEffectRemoved(){}
	void RemoveStatusEffectInternal(UStatusEffect* StatusEffect);
	
	bool AreMultipleVisible(AActor* Target, ETraceTypeQuery TraceType, const FVector& TraceStart,
							TArray<FVector>& RemainingEnds, int32 RequiredPositiveTests) const;

	UFUNCTION(BlueprintCallable, meta=(NewEffectType="/Script/MAProject.StatusEffect"))
	void ReceiveStatusEffect(TSubclassOf<UStatusEffect> NewEffectType);
	UFUNCTION(BlueprintCallable, meta=(NewEffectType="/Script/MAProject.StatusEffect"))
	void RemoveStatusEffect(TSubclassOf<UStatusEffect> EffectType);
	UFUNCTION(BlueprintCallable)
	void RegisterRelevantMeshes(const TArray<USkeletalMeshComponent*>& NewMeshes, bool AddToBaseMesh = false,
		bool ForceUpdate = false);



#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = Debugging)
	bool bIsDebugging = false;
#endif

private:
	UPROPERTY()
	APlayerController* CameraPlayerController;
};
