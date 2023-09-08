// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MovementTarget.generated.h"

class USphereComponent;

struct FSetMovementTargetKey final
{
	friend class AOpponentController;
private:
	FSetMovementTargetKey(){};
};

UCLASS()
class AMovementTarget : public AActor
{
	GENERATED_BODY()
	
public:	
	AMovementTarget();

	virtual void Tick(float DeltaTime) override;
	
	float GetBlendTime() const { return BlendTime; }
	FVector GetMovementTargetLocation() const { return TargetLocation; }
	AActor* GetTargetActor() const { return TargetActor; }

	void SetBlendTime(float NewBlendTime, FSetMovementTargetKey Key) { BlendTime = NewBlendTime; }
	void SetMovementTargetLocation(const FVector& NewTargetLocation, FSetMovementTargetKey Key);
	void SetTargetActor(AActor* NewTargetActor, FSetMovementTargetKey Key){ TargetActor = NewTargetActor; }

#if WITH_EDITORONLY_DATA
	void ToggleDebugging(){ bIsDebugging = !bIsDebugging;}
	void SetIsDebugging(bool IsDebugging){ bIsDebugging = IsDebugging; }
	bool GetIsDebugging() const { return bIsDebugging; }
#endif
	
protected:
	
	float BlendTime;
	FVector TargetLocation;

#if WITH_EDITORONLY_DATA
	bool bIsDebugging = false;
#endif
	
	UPROPERTY()
	AActor* TargetActor;
	
	UPROPERTY(EditAnywhere)
	float MaxVelocity;

	UPROPERTY(EditAnywhere)
	USphereComponent* SphereComponent;

};
