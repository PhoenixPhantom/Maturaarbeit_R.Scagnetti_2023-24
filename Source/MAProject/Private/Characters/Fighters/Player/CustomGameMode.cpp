// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Player/CustomGameMode.h"

#include "Characters/Fighters/Player/PlayerPartyController.h"
#include "Utility/Savegame/ReadWriteHelpers.h"

ACustomGameMode::ACustomGameMode(): PlayerSetupData(nullptr)
{
}

APlayerController* ACustomGameMode::SpawnPlayerController(ENetRole InRemoteRole, const FString& Options)
{
	APlayerController* PlayerController = Super::SpawnPlayerController(InRemoteRole, Options);
	APlayerPartyController* PartyController = Cast<APlayerPartyController>(PlayerController);
	if(IsValid(PartyController) && PlayerSetupData != nullptr)
	{
		PartyController->SetPawnStartTransform(PlayerSetupData->Transform, FSetPawnStartTransformKey());
		UReadWriteHelpers::WriteToTarget(PlayerController, PlayerSetupData->SerializedData);
	}
	
	return PlayerController;
}
