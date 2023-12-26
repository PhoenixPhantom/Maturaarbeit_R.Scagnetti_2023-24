// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "TargetInformationComponent.generated.h"

struct FSetCanBeTargetedKey final
{
	friend class AFighterCharacter;
	friend class APlayerCharacter;
private:
	FSetCanBeTargetedKey(){}
};

struct FSetTargetStateKey final
{
	friend class APlayerCharacter;
private:
	FSetTargetStateKey(){}
};

UCLASS(ClassGroup=(Custom))
class MAPROJECT_API UTargetInformationComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTargetInformationComponent();

	void SetCanBeTargeted(bool CanBeTargeted, FSetCanBeTargetedKey){ bCanBeTargeted = CanBeTargeted; }

	float GetTargetPriority() const { return TargetPriority; }
	void AddTargetingEntity(AController* TargetingController, FSetTargetStateKey);
	void RemoveTargetingEntity(AController* NonTargetingController, FSetTargetStateKey);
	bool IsTargetOf(AController* TargetingController) const { return TargetingControllers.Contains(TargetingController); }
	bool GetCanBeTargeted() const { return bCanBeTargeted; }

protected:
	uint8 bCanBeTargeted:1;

	UPROPERTY()
	TArray<AController*> TargetingControllers;
	
	UPROPERTY(EditAnywhere, meta=(Units="%", ToolTip="The priority with which the actor will be targeted. An actor with priority 2 is double as likely to be targeted as one with 1)"))
	float TargetPriority;
};
