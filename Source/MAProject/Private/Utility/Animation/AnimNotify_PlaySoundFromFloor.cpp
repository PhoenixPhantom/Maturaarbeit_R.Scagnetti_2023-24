// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/Animation/AnimNotify_PlaySoundFromFloor.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void FSoundConfig::PlaySoundAtLocation(UWorld* World, const FVector& PlayLocation) const
{
	if(IsValid(Sound)) UGameplayStatics::PlaySoundAtLocation(World, Sound, PlayLocation, VolumeMultiplier, PitchMultiplier);
}

UAnimNotify_PlaySoundFromFloor::UAnimNotify_PlaySoundFromFloor() : ScanLength(100.f)
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
	SoundResponseConfig.GetDefaultObject()->GetPhysicsResponses().FindRef(PhysicalSurfaceType).PlaySoundAtLocation(MeshComp->GetWorld(), TraceStart);
}
