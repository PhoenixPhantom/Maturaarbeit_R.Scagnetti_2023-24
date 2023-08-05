// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/Animation/AnimNotify_PlaySoundFromFloor.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

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

UAnimNotify_PlaySoundFromFloor::UAnimNotify_PlaySoundFromFloor() : ScanLength(20.f), VolumeMultiplier(1.f),
	PitchMultiplier(1.f)
{
}

void UAnimNotify_PlaySoundFromFloor::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                            const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	const FVector TraceStart = MeshComp->GetSocketLocation(ScanStartSocket);
	FHitResult HitResult;
	UKismetSystemLibrary::LineTraceSingle(MeshComp->GetWorld(), TraceStart, TraceStart + FVector(0.f, 0.f, -ScanLength),
		UEngineTypes::ConvertToTraceType(ECC_Visibility), true, {},
		EDrawDebugTrace::None, HitResult, true);
	
	if(!HitResult.bBlockingHit) return;
	const EPhysicalSurface PhysicalSurfaceType = UGameplayStatics::GetSurfaceType(HitResult);

	if(!IsValid(SoundResponseConfig.Get())) return;
	SoundResponseConfig.GetDefaultObject()->GetPhysicsResponses().FindRef(PhysicalSurfaceType).
		PlaySoundAtLocation(MeshComp->GetWorld(), HitResult.Location, VolumeMultiplier, PitchMultiplier);
}
