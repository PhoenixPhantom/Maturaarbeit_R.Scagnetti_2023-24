// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterStats.h"
#include "Characters/GeneralCharacter.h"
#include "FighterCharacter.generated.h"

struct FMeleeControlsKey final
{
private:
	FMeleeControlsKey(){}
};

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

	void ActivateMeleeBones(const TArray<FName>& BonesToEnable, FMeleeControlsKey Key);
	void DeactivateMeleeBones(const TArray<FName>& BonesToDisable, FMeleeControlsKey Key);
	

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	FCharacterBaseStats BaseStats;
	FCharacterStats* CharacterStats;

	UPROPERTY()
	TArray<AActor*> RecentlyDamagedActors;
	TArray<FName> MeleeEnabledBones;
	
	UFUNCTION()
	void OnMeshOverlapEvent(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp,  int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
};
