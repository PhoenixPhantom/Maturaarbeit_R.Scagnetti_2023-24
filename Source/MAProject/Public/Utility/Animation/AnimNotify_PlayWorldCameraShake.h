// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_PlayWorldCameraShake.generated.h"

/**
 * 
 */
UCLASS()
class MAPROJECT_API UAnimNotify_PlayWorldCameraShake : public UAnimNotify
{
	GENERATED_BODY()
public:
	UAnimNotify_PlayWorldCameraShake();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
protected:
	UPROPERTY(EditAnywhere, Category = UserExperience)
	TSubclassOf<UCameraShakeBase> CameraShake;

	UPROPERTY(EditAnywhere)
	FVector ShakeSourceOffset;

	UPROPERTY(EditAnywhere)
	float ShakeDistance;
};
