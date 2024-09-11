// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "SoundscapeSubsystem.h"
#include "GlobalSoundManager.generated.h"

class ATriggerBase;
class UBoxComponent;

struct FSetCombatStateKey final
{
	friend class ACombatManager;
private:
	FSetCombatStateKey(){}
};

UCLASS()
class MAPROJECT_API AGlobalSoundManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGlobalSoundManager();

	void EnterCombatState(FSetCombatStateKey) const;
	void EndCombatState(FSetCombatStateKey) const;

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY()
	USoundscapeSubsystem* Soundscape;

	UPROPERTY()
	AController* PlayerController;

	UPROPERTY()
	ATriggerBase* ActiveVolume;

	UPROPERTY(EditAnywhere)
	FSoundscapePaletteCollection RelevantPalettes;

	UPROPERTY(EditAnywhere)
	TMap<FGameplayTag, FColor> TagColors;
	
	UPROPERTY(EditAnywhere)
	TMap<ATriggerBase*, FGameplayTag> WorldAreaTags;

	UPROPERTY(EditAnywhere)
	FGameplayTag CombatTag;

	UFUNCTION()
	void OnBoundingVolumeOverlap(AActor* OverlappedActor, AActor* OtherActor);
};
