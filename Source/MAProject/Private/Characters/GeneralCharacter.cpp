// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/GeneralCharacter.h"
#include "MotionWarpingComponent.h"

// Sets default values
AGeneralCharacter::AGeneralCharacter() : FieldOfView(90.f)
{
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComp"));

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionObjectType(ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECR_Overlap);

	GetMesh()->ComponentTags.Add(DamageableVolumeTag);
	
}

void AGeneralCharacter::GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	if(!IsValid(GetMesh())) return Super::GetActorEyesViewPoint(OutLocation, OutRotation);
	const FName HeadSocket = *(BonePrefix + "-HeadSocket");
	OutLocation = GetMesh()->GetSocketLocation(HeadSocket);
	OutRotation = GetMesh()->GetSocketRotation(HeadSocket);
}

// Called when the game starts or when spawned
void AGeneralCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

