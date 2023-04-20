// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/GeneralCharacter.h"

// Sets default values
AGeneralCharacter::AGeneralCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGeneralCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGeneralCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AGeneralCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

