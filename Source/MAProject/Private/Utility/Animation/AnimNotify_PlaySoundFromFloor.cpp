// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/Animation/AnimNotify_PlaySoundFromFloor.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Utility/Sound/SoundResponseConfigs.h"

UAnimNotify_PlaySoundFromFloor::UAnimNotify_PlaySoundFromFloor() : ScanLength(20.f), VolumeMultiplier(1.f),
                                                                   PitchMultiplier(1.f)
{
}

void UAnimNotify_PlaySoundFromFloor::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                            const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	FVector TraceStart;
	if(MeshComp->DoesSocketExist(ScanStartSocket)) TraceStart = MeshComp->GetSocketLocation(ScanStartSocket);
	else TraceStart = MeshComp->GetComponentLocation();
	FHitResult HitResult;
	UKismetSystemLibrary::LineTraceSingle(MeshComp->GetWorld(), TraceStart, TraceStart + FVector(0.f, 0.f, -ScanLength),
		UEngineTypes::ConvertToTraceType(ECC_Camera), true, {},
		EDrawDebugTrace::None, HitResult, true);
	
	if(!HitResult.bBlockingHit) return;
	const EPhysicalSurface PhysicalSurfaceType = UGameplayStatics::GetSurfaceType(HitResult);

	if(!IsValid(SoundResponseConfig.Get())) return;
	const FSoundConfig SoundConfig = SoundResponseConfig.GetDefaultObject()->GetPhysicsResponses().FindRef(PhysicalSurfaceType);
	SoundConfig.PlaySoundAtLocation(MeshComp->GetWorld(), HitResult.Location, VolumeMultiplier, PitchMultiplier);
}
