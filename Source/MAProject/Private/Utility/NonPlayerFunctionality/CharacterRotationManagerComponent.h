// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "CharacterRotationManagerComponent.generated.h"


class AAIController;
class AOpponentCharacter;


enum class ECharacterRotationMode
{
	OrientToMovement,
	OrientToTarget,
	FlickBack
};


UCLASS(ClassGroup=(Custom))
class MAPROJECT_API UCharacterRotationManagerComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UCharacterRotationManagerComponent();

	void SwitchToOptimal(const FVector& TargetLocation);
	void SetRotationMode(ECharacterRotationMode NewRotationMode, bool StoreForFlickBack = false, AActor* NewTarget = nullptr);

protected:
	ECharacterRotationMode CharacterRotationMode;
	ECharacterRotationMode StoredCharacterRotationMode;
	
	UPROPERTY()
	AOpponentCharacter* OpponentCharacter;

	UPROPERTY()
	AAIController* OpponentController;

	UPROPERTY()
	AActor* StoredTarget;
	
	virtual void BeginPlay() override;
};
