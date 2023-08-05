// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimNotify_PlaySoundFromFloor.generated.h"

USTRUCT()
struct FSoundConfig
{
	GENERATED_BODY()

	FSoundConfig() : Sound(nullptr), SoundAttenuation(nullptr), VolumeMultiplier(1.f), PitchMultiplier(1.f){}

	UPROPERTY(EditAnywhere)
	USoundBase* Sound;

	UPROPERTY(EditAnywhere)
	USoundAttenuation* SoundAttenuation;
	
	UPROPERTY(EditAnywhere)
	float VolumeMultiplier;
	UPROPERTY(EditAnywhere)
	float PitchMultiplier;

	bool operator==(const FSoundConfig& SoundConfig) const;
	
	FORCEINLINE void PlaySoundAtLocation(UWorld* World, const FVector& PlayLocation, float AdditionalVolumeMultiplier,
		float AdditionalPitchMultiplier) const;
};

UCLASS(Blueprintable, BlueprintType)
class MAPROJECT_API UPhysicsSoundResponseConfig : public UObject
{
	GENERATED_BODY()
public:
	const TMap<TEnumAsByte<EPhysicalSurface>, FSoundConfig>& GetPhysicsResponses(){ return PhysicsResponses; }
protected:
	UPROPERTY(EditAnywhere)                                
	TMap<TEnumAsByte<EPhysicalSurface>, FSoundConfig> PhysicsResponses;
};

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
