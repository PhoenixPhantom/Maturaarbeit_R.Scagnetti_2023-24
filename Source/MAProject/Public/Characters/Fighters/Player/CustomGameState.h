// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CustomGameState.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSaveGame, Warning, All);

class UWorldStateSaveGame;
/**
 * 
 */
UCLASS()
class MAPROJECT_API ACustomGameState : public AGameStateBase
{
	GENERATED_BODY()
public:
	static const FString WorldSaveGameName;
	ACustomGameState();
	virtual void ReceivedGameModeClass() override;

	void LoadSaveGame();
	void WriteSaveGame();
	
protected:

	UPROPERTY()
	UWorldStateSaveGame* WorldSaveGame;
};
