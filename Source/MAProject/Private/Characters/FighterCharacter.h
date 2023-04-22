// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/GeneralCharacter.h"
#include "FighterCharacter.generated.h"

/**
 * 
 */
UCLASS()
class AFighterCharacter : public AGeneralCharacter
{
	GENERATED_BODY()

public:
	AFighterCharacter();
	virtual void Tick(float DeltaSeconds) override;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
		AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;

	TArray<FName> MeleeEnabledBones;
	
	UFUNCTION()
	void OnMeshOverlapEvent(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp,  int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
};
