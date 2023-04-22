// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/FighterCharacter.h"

AFighterCharacter::AFighterCharacter()
{
	GetMesh()->SetGenerateOverlapEvents(true);
	GetMesh()->OnComponentBeginOverlap.AddDynamic(this, &AFighterCharacter::OnMeshOverlapEvent);
}

void AFighterCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

float AFighterCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void AFighterCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AFighterCharacter::OnMeshOverlapEvent(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}
