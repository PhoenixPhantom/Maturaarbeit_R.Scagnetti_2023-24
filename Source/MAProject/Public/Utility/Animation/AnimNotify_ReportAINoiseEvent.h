// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_ReportAINoiseEvent.generated.h"

class UPhysicsAILoudnessResponseConfig;
/**
 * 
 */
UCLASS()
class MAPROJECT_API UAnimNotify_ReportAINoiseEvent : public UAnimNotify
{
	GENERATED_BODY()
public:
	UAnimNotify_ReportAINoiseEvent();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
protected:
	UPROPERTY(EditAnywhere)
	FName Socket;

	UPROPERTY(EditAnywhere)
	bool bScanForFloor;

	UPROPERTY(EditAnywhere, meta=(EditCondition="bScanForFloor", EditConditionHides))
	float ScanLength;
	UPROPERTY(EditAnywhere, meta=(EditCondition="bScanForFloor", EditConditionHides))
	TSubclassOf<UPhysicsAILoudnessResponseConfig> SoundResponseConfig;

	
	UPROPERTY(EditAnywhere)
	float AILoudness;
	UPROPERTY(EditAnywhere, AdvancedDisplay)
	float MaxSoundRange;
	UPROPERTY(EditAnywhere, AdvancedDisplay)
	FName SoundTag;
};
