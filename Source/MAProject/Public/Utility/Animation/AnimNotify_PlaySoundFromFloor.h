// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimNotify_PlaySoundFromFloor.generated.h"

class UPhysicsSoundResponseConfig;
/**
 * 
 */
UCLASS()
class MAPROJECT_API UAnimNotify_PlaySoundFromFloor : public UAnimNotify
{
	GENERATED_BODY()
public:
	UAnimNotify_PlaySoundFromFloor();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
protected:
	UPROPERTY(EditAnywhere)
	FName ScanStartSocket;
	
	UPROPERTY(EditAnywhere)
	float ScanLength;

	UPROPERTY(EditAnywhere)
	float VolumeMultiplier;
	UPROPERTY(EditAnywhere)
	float PitchMultiplier;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UPhysicsSoundResponseConfig> SoundResponseConfig;
};
