// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/Animation/AnimNotify_PlayWorldCameraShake.h"

#include "Kismet/GameplayStatics.h"

UAnimNotify_PlayWorldCameraShake::UAnimNotify_PlayWorldCameraShake(): ShakeDistance(10000.f)
{
}

void UAnimNotify_PlayWorldCameraShake::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                              const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	UGameplayStatics::PlayWorldCameraShake(MeshComp->GetWorld(), CameraShake,
			MeshComp->GetComponentLocation() + ShakeSourceOffset,0.f, ShakeDistance);
}
