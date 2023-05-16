// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/CombatManager.h"

#include "Characters/Fighters/Player/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
ACombatManager::ACombatManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

APlayerCharacter* ACombatManager::GetPlayerCharacter() const
{
	return CastChecked<APlayerCharacter>(PlayerController->GetCharacter());
}

// Called when the game starts or when spawned
void ACombatManager::BeginPlay()
{
	Super::BeginPlay();

	//There can only be one combat manager at any given time to prevent problems
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACombatManager::StaticClass(), Actors);
	check(Actors.Num() == 1 && Actors[0] == this);

	PlayerController = GetWorld()->GetFirstPlayerController();	
}

