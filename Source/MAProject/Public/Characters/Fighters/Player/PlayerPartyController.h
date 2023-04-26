// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Fighters/CharacterStats.h"
#include "GameFramework/PlayerController.h"
#include "PlayerPartyController.generated.h"

class APlayerCharacter;

struct FSetPawnStartTransformKey final
{
	friend class ACustomGameMode;
private:
	FSetPawnStartTransformKey(){};
};

UCLASS()
class MAPROJECT_API APlayerPartyController : public APlayerController
{
	GENERATED_BODY()
public:
	APlayerPartyController();
	void SetPawnStartTransform(FTransform Transform, FSetPawnStartTransformKey Key){ PawnStartTransform = Transform; }
protected:
	FTransform PawnStartTransform;

	FCharacterStats PartyMemberStats;
	UPROPERTY(EditAnywhere)
	TSubclassOf<APlayerCharacter> PartyMemberClass;
	UPROPERTY(SaveGame)
	FSavableCharacterModifiers PartyMemberModifiers;

	virtual void BeginPlay() override;
};
