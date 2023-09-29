// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Fighters/CharacterStats.h"
#include "GameFramework/PlayerController.h"
#include "GenericTeamAgentInterface.h"
#include "PlayerPartyController.generated.h"

class APlayerCharacter;
class ACombatManager;

struct FSetPawnStartTransformKey final
{
	friend class ACustomGameMode;
private:
	FSetPawnStartTransformKey(){};
};

struct FSetPlayerUserSettingsKey final
{
	friend class USettingsMenuWidget;
private:
	FSetPlayerUserSettingsKey(){};
};
 
USTRUCT()
struct FPlayerUserSettings
{
	GENERATED_BODY()
public:
	FPlayerUserSettings() : CameraZoomSpeed(10.f), bPreferClosestTarget(true){}
	
	UPROPERTY(VisibleAnywhere, SaveGame)
	float CameraZoomSpeed;

	//This defines weather the player character always attacks the closest target
	//or always attacks the one most centered in view
	UPROPERTY(VisibleAnywhere, SaveGame)
	bool bPreferClosestTarget;
};

UCLASS()
class MAPROJECT_API APlayerPartyController : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
public:
	APlayerPartyController();
	void SetPawnStartTransform(FTransform Transform, FSetPawnStartTransformKey Key){ PawnStartTransform = Transform; }
	const FPlayerUserSettings& GetPlayerUserSettings() const { return PlayerUserSettings; }
	void SetPlayerUserSettings(const FPlayerUserSettings& NewPlayerUserSettings, FSetPlayerUserSettingsKey Key){ PlayerUserSettings = NewPlayerUserSettings; };
	
	virtual FGenericTeamId GetGenericTeamId() const override { return 0; }
	
protected:
	FTransform PawnStartTransform;
	FCharacterStats PartyMemberStats;
	
	UPROPERTY()
	ACombatManager* CombatManager;
	
	UPROPERTY(EditAnywhere, SaveGame)
	TSubclassOf<APlayerCharacter> PartyMemberClass;
	UPROPERTY(SaveGame)
	FSavableCharacterModifiers PartyMemberModifiers;
	UPROPERTY(SaveGame)
	FPlayerUserSettings PlayerUserSettings;

	virtual void BeginPlay() override;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = Debugging)
	bool bIsDebugging = false;

	UFUNCTION(CallInEditor, Category = Debugging)
	void ToggleDebugging();
#endif
};
