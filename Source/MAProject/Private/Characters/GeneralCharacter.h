// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <tiffio.h>

#include "CoreMinimal.h"
#include "InputManagement.h"
#include "GameFramework/Character.h"
#include "GeneralCharacter.generated.h"

class USuckToTargetComponent;
class UMotionWarpingComponent;



UCLASS(meta=(PrioritizeCategories = "Debugging Combat OpponentCharacter"))
class AGeneralCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGeneralCharacter(const FObjectInitializer& ObjectInitializer);

	virtual float GetFieldOfView() const { unimplemented(); return 0.f; }
	
	/**
	 * @brief Get weather an input of the given can override the currently active one or not
	 * @param Type the input to check
	 * @return Weather the current input can be overridden by Type
	 */
	bool CanOverrideCurrentInput(EInputType Type) const { return AcceptedInputs.CanOverrideCurrentInput(Type); }
	const FAcceptedInputs& GetAcceptedInputs() const { return AcceptedInputs; }

	virtual void GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;

	virtual void Tick(float DeltaSeconds) override;

#if WITH_EDITORONLY_DATA
	void SetIsDebugging(bool IsDebugging);
	bool GetIsDebugging() const { return bIsDebugging; }
#endif

protected:
	FAcceptedInputs AcceptedInputs;
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

	UPROPERTY(EditAnywhere, Category = Animation)
	USuckToTargetComponent* SuckToTargetComponent;

	void FadeMeshWithCameraDistance();
	void SetMeshesOpacity(float DesiredOpacity);

	UFUNCTION(BlueprintCallable)
	void RegisterRelevantMeshes(const TArray<USkeletalMeshComponent*>& NewMeshes, bool AddToBaseMesh = false,
		bool ForceUpdate = false);

	bool AreMultipleVisible(AActor* Target, ETraceTypeQuery TraceType, const FVector& TraceStart,
	                        TArray<FVector>& RemainingEnds, int32 RequiredPositiveTests) const;

	virtual void BeginPlay() override;


#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = Debugging)
	bool bIsDebugging = false;
#endif

private:
	UPROPERTY()
	APlayerController* CameraPlayerController;
};
