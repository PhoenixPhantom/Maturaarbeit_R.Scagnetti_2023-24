// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Player/PlayerPartyController.h"

#include "Characters/Fighters/Player/PlayerCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

APlayerPartyController::APlayerPartyController() : PawnStartTransform(FVector(NAN))
{
}

void APlayerPartyController::BeginPlay()
{

	Super::BeginPlay();

	const APlayerCharacter* TargetCharacter = CastChecked<APlayerCharacter>(PartyMemberClass.GetDefaultObject());
	PartyMemberStats.FromBase(TargetCharacter->GetCharacterBaseStats(), PartyMemberModifiers, GetWorld());

	FTransform TargetTransform;
	//If we have already a saved location
	if(PawnStartTransform.IsValid())
	{
		TargetTransform = PawnStartTransform;
	}
	//if this is the first character that is spawned (ever), it should be spawned at the PlayerStart
	//(which can optionally be marked with the tag "ActiveSpawn")
	else
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), FoundActors);
		check(!FoundActors.IsEmpty());
		APlayerStart* ChosenPlayerStart = nullptr;
		for(AActor* Actor : FoundActors)
		{
			ChosenPlayerStart = CastChecked<APlayerStart>(Actor);
			if(ChosenPlayerStart->PlayerStartTag == "ActiveSpawn") break;
		}
		TargetTransform = ChosenPlayerStart->GetTransform();
		UE_LOG(LogTemp, Log, TEXT("Spawn transform is: %s"), *TargetTransform.ToString());
	}

	APlayerCharacter* NewCharacter = GetWorld()->SpawnActorDeferred<APlayerCharacter>(PartyMemberClass.Get(),
		TargetTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

	NewCharacter->PreSpawnSetup(&PartyMemberStats, &PlayerUserSettings, FPreSpawnSetupKey());
	NewCharacter->FinishSpawning(TargetTransform);
	
	Possess(NewCharacter);
	GetPawn()->EnableInput(this); //Input seems to be disabled by default	
}
