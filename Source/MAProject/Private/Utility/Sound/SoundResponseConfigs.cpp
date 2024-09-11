// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/Sound/SoundResponseConfigs.h"

#include "Kismet/GameplayStatics.h"


bool FSoundConfig::operator==(const FSoundConfig& SoundConfig) const
{
	return Sound == SoundConfig.Sound && VolumeMultiplier == SoundConfig.VolumeMultiplier &&
		PitchMultiplier == SoundConfig.PitchMultiplier;
}

void FSoundConfig::PlaySoundAtLocation(UWorld* World, const FVector& PlayLocation, float AdditionalVolumeMultiplier,
	float AdditionalPitchMultiplier) const
{
	if(IsValid(Sound)) UGameplayStatics::PlaySoundAtLocation(World, Sound, PlayLocation,
		VolumeMultiplier * AdditionalVolumeMultiplier, PitchMultiplier * AdditionalPitchMultiplier,
		0.f, SoundAttenuation);

}