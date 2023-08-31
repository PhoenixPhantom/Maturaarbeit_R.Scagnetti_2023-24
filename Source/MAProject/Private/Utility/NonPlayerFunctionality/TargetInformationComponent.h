// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "TargetInformationComponent.generated.h"

struct FSetCanBeTargetedKey final
{
	friend class AFighterCharacter;
private:
	FSetCanBeTargetedKey(){}
};

struct FSetTargetStateKey final
{
	friend class APlayerCharacter;
private:
	FSetTargetStateKey(){}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChangeTargetStateDelegate, bool, IsCurrentTarget);

UCLASS(ClassGroup=(Custom))
class MAPROJECT_API UTargetInformationComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	FOnChangeTargetStateDelegate OnChangeTargetState;
	// Sets default values for this component's properties
	UTargetInformationComponent();

	void SetCanBeTargeted(bool CanBeTargeted, FSetCanBeTargetedKey){ bCanBeTargeted = CanBeTargeted; }

	float GetTargetPriority() const { return TargetPriority; }
	void SetIsCurrentTarget(bool TargetState, FSetTargetStateKey);
	bool GetIsCurrentTarget() const { return bIsCurrentTarget; }
	bool GetCanBeTargeted() const { return bCanBeTargeted; }

protected:
	UPROPERTY(EditAnywhere, meta=(Units="%", ToolTip="The priority with which the actor will be targeted. An actor with priority 2 is double as likely to be targeted as one with 1)"))
	float TargetPriority;

	uint8 bIsCurrentTarget:1;
	uint8 bCanBeTargeted:1;	
};
