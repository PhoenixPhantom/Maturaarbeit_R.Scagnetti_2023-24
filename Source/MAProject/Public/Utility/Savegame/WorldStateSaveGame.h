// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "WorldStateSaveGame.generated.h"

USTRUCT()
struct FGeneralActorSaveData
{
	GENERATED_BODY()
public:
	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	TArray<uint8> SerializedData;
};

USTRUCT()
struct FNonPlayerSaveData : public FGeneralActorSaveData
{
	GENERATED_BODY()
public:
	FNonPlayerSaveData() : FGeneralActorSaveData(), ActorUniqueWorldID(0){}
	UPROPERTY()
	uint64 ActorUniqueWorldID;
};

UCLASS()
class MAPROJECT_API UWorldStateSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UWorldStateSaveGame(){};

	UPROPERTY()
	FGeneralActorSaveData PlayerData;

	UPROPERTY()
	TArray<FNonPlayerSaveData> SavedActors;
	
};
