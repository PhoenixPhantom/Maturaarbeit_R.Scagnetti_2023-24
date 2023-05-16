// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "TargetInformationComponent.generated.h"


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

	float GetTargetPriority() const { return TargetPriority; }
	void SetTargetState(bool TargetState, FSetTargetStateKey Key) { bIsCurrentTarget = TargetState; OnChangeTargetState.Broadcast(bIsCurrentTarget); }
	bool GetTargetState() const { return bIsCurrentTarget; }

protected:
	UPROPERTY(EditAnywhere, meta=(Units="%", ToolTip="The priority with which the actor will be targeted. An actor with priority 2 is double as likely to be targeted as one with 1)"))
	float TargetPriority;

	bool bIsCurrentTarget;
};
