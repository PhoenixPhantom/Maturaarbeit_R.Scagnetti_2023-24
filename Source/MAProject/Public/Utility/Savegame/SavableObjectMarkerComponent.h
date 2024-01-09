// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SavableObjectMarkerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDataSavedDelegate);

struct FSetUniqueWorldIdKey final
{
	friend class ASavableObjectIDGenerator;
private:
	FSetUniqueWorldIdKey(){};
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MAPROJECT_API USavableObjectMarkerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	static const uint64 PlayerControllerID;
	USavableObjectMarkerComponent() : UniqueWorldID(0){}

	FOnDataSavedDelegate OnActorLoaded;

	//Gets the custom generated unique world ID (used to exactly identify the actor in the save file)
	uint64 GetUniqueWorldID() const { return UniqueWorldID; };
	//Sets the custom generated unique world ID (can only be used by the in-editor generator)
	void SetUniqueWorldID(uint64 NewId, FSetUniqueWorldIdKey Key){ UniqueWorldID = NewId; }
	
protected:
	UPROPERTY(VisibleAnywhere)
	uint64 UniqueWorldID;
};
