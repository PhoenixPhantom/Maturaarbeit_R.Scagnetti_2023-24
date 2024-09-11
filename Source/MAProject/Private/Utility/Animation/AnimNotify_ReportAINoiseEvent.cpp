// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/Animation/AnimNotify_ReportAINoiseEvent.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Perception/AISense_Hearing.h"
#include "Utility/Sound/SoundResponseConfigs.h"

UAnimNotify_ReportAINoiseEvent::UAnimNotify_ReportAINoiseEvent(): bScanForFloor(false), ScanLength(20.f),
                                                                  AILoudness(1.f), MaxSoundRange(0.f)
{
	bShouldFireInEditor = false;
}

void UAnimNotify_ReportAINoiseEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                            const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	FVector SoundLocation;
	if(MeshComp->DoesSocketExist(Socket)) SoundLocation = MeshComp->GetSocketLocation(Socket);
	else SoundLocation = MeshComp->GetComponentLocation();

	float ResultingLoudness = AILoudness;
	if(bScanForFloor)
	{
		FHitResult HitResult;
		UKismetSystemLibrary::LineTraceSingle(MeshComp->GetWorld(), SoundLocation, 
		SoundLocation + FVector(0.f, 0.f, -ScanLength),
			UEngineTypes::ConvertToTraceType(ECC_Camera), true, {},
			EDrawDebugTrace::None, HitResult, true);
	
		if(!HitResult.bBlockingHit) return;
		SoundLocation = HitResult.Location;
		const EPhysicalSurface PhysicalSurfaceType = UGameplayStatics::GetSurfaceType(HitResult);

		if(!IsValid(SoundResponseConfig.Get())) return;
		ResultingLoudness *= SoundResponseConfig.GetDefaultObject()->GetPhysicsResponses().FindRef(PhysicalSurfaceType);
	}

	//for some reason we cannot use GetWorld() directly as that always returns nullptr
	UAISense_Hearing::ReportNoiseEvent(MeshComp->GetWorld(), SoundLocation, ResultingLoudness,
		MeshComp->GetOwner()->GetInstigator(), MaxSoundRange, SoundTag);
}
