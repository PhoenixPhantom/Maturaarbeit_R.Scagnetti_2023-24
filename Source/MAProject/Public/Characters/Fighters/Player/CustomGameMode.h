// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Utility/Savegame/WorldStateSaveGame.h"
#include "CustomGameMode.generated.h"

struct FSetPlayerSetupDataKey final
{
	friend class ACustomGameState;
private:
	FSetPlayerSetupDataKey(){};
};

/**
 * 
 */
UCLASS()
class MAPROJECT_API ACustomGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	ACustomGameMode();
	void SetPlayerSetupData(FGeneralActorSaveData* NewPlayerData, FSetPlayerSetupDataKey Key){ PlayerSetupData = NewPlayerData; }
	virtual APlayerController* SpawnPlayerController(ENetRole InRemoteRole, const FString& Options) override;
protected:
	FGeneralActorSaveData* PlayerSetupData;
};
