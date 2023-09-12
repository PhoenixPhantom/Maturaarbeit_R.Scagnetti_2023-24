// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SoundResponseConfigs.generated.h"


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
	
	void PlaySoundAtLocation(UWorld* World, const FVector& PlayLocation,
		float AdditionalVolumeMultiplier = 1.f, float AdditionalPitchMultiplier = 1.f) const;
};


//Get the corresponding sound for every EPhysicalSurface type as defined
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

//Get the corresponding sound for every EPhysicalSurface type as defined
UCLASS(Blueprintable, BlueprintType)
class MAPROJECT_API UPhysicsAILoudnessResponseConfig : public UObject
{
	GENERATED_BODY()
public:
	const TMap<TEnumAsByte<EPhysicalSurface>, float>& GetPhysicsResponses(){ return PhysicsAILoudnessResponses; }
protected:
	UPROPERTY(EditAnywhere)                                
	TMap<TEnumAsByte<EPhysicalSurface>, float> PhysicsAILoudnessResponses;
};



//Get the corresponding sound for every bone as defined
//(if nothing is set for the bone, it will use the same sound as it's parent)
UCLASS(Blueprintable, BlueprintType)
class MAPROJECT_API UBoneSoundResponseConfig : public UObject
{
	GENERATED_BODY()
public:
	const TMap<FName, FSoundConfig>& GetBoneResponses(){ return BoneResponses; }
protected:
	UPROPERTY(EditAnywhere)                                
	TMap<FName, FSoundConfig> BoneResponses;
};
