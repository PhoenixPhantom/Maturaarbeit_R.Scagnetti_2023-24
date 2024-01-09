// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Player/CustomGameState.h"

#include "EngineUtils.h"
#include "Characters/Fighters/Player/CustomGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/SavePackage.h"
#include "Utility/Savegame/ReadWriteHelpers.h"
#include "Utility/Savegame/SavableObjectMarkerComponent.h"
#include "Utility/Savegame/WorldStateSaveGame.h"

DEFINE_LOG_CATEGORY(LogSaveGame);

const FString ACustomGameState::WorldSaveGameName = "WorldSaveGame";

ACustomGameState::ACustomGameState() : WorldSaveGame(nullptr)
{
}

void ACustomGameState::ReceivedGameModeClass()
{
	Super::ReceivedGameModeClass();
	if(UGameplayStatics::DoesSaveGameExist(WorldSaveGameName, 0))
	{
		WorldSaveGame = Cast<UWorldStateSaveGame>(UGameplayStatics::LoadGameFromSlot(WorldSaveGameName, 0));
		LoadSaveGame();
	}
}

void ACustomGameState::LoadSaveGame()
{
	if(!IsValid(WorldSaveGame))
	{
		UE_LOG(LogSaveGame, Warning, TEXT("Failed to load SaveGame Data."));
		return;
	}

	//The player controller and all its data have to be stored separately because they require
	//special loading procedures
	CastChecked<ACustomGameMode>(AuthorityGameMode)->SetPlayerSetupData(&WorldSaveGame->PlayerData, FSetPlayerSetupDataKey());

	//Iterate the entire world of actors
	TArray<AActor*> WorldActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), WorldActors);
	for(AActor* Actor : WorldActors)
	{
		//Only interested in Savable actors
		if(!IsValid(Actor)) continue;
		UActorComponent* ObjectComponent = Actor->GetComponentByClass(USavableObjectMarkerComponent::StaticClass());
		if(!IsValid(ObjectComponent)) continue;
		const USavableObjectMarkerComponent* CallRef = CastChecked<USavableObjectMarkerComponent>(ObjectComponent);

		for(FNonPlayerSaveData ActorSaveData : WorldSaveGame->SavedActors)
		{
			if(ActorSaveData.ActorUniqueWorldID == CallRef->GetUniqueWorldID())
			{
				Actor->SetActorTransform(ActorSaveData.Transform);
				UReadWriteHelpers::WriteToTarget(Actor, ActorSaveData.SerializedData);				
				CallRef->OnActorLoaded.Broadcast();

				break;
			}
		}
	}
}

void ACustomGameState::WriteSaveGame()
{
	if(!UGameplayStatics::DoesSaveGameExist(WorldSaveGameName, 0))
	{
		WorldSaveGame = Cast<UWorldStateSaveGame>(UGameplayStatics::CreateSaveGameObject(UWorldStateSaveGame::StaticClass()));
		UE_LOG(LogSaveGame, Log, TEXT("Created New SaveGame Data."));
	}

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	check(IsValid(PlayerController));

	//The player controller and all its data have to be stored separately because they require
	//special loading procedures
	WorldSaveGame->PlayerData.Transform = PlayerController->GetPawn()->GetActorTransform();
	UReadWriteHelpers::ReadFromTarget(PlayerController, WorldSaveGame->PlayerData.SerializedData);
	
	WorldSaveGame->SavedActors.Empty();
	//Iterate the entire world of actors
	TArray<AActor*> WorldActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), WorldActors);
	for(AActor* Actor : WorldActors)
	{
		if(!IsValid(Actor)) continue;
		UActorComponent* SavableObjectComponent = Actor->GetComponentByClass(USavableObjectMarkerComponent::StaticClass());
		if(!IsValid(SavableObjectComponent)) continue;


		FNonPlayerSaveData ActorSaveData;
		ActorSaveData.ActorUniqueWorldID = CastChecked<USavableObjectMarkerComponent>(SavableObjectComponent)->GetUniqueWorldID();
		ActorSaveData.Transform = Actor->GetTransform();
		UReadWriteHelpers::ReadFromTarget(Actor, ActorSaveData.SerializedData);

		WorldSaveGame->SavedActors.Add(ActorSaveData);
	}
	
	UGameplayStatics::AsyncSaveGameToSlot(WorldSaveGame, WorldSaveGameName, 0);
}

