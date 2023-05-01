// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterStats.h"
#include "Characters/GeneralCharacter.h"
#include "FighterCharacter.generated.h"

struct FMeleeControlsKey final
{
	friend class UMeleeAttackNotifyState;
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

	void ActivateMeleeBones(const TArray<FName>& BonesToEnable, bool StartEmpty, FMeleeControlsKey Key);
	void DeactivateMeleeBones(const TArray<FName>& BonesToDisable, bool RefreshHitActors, FMeleeControlsKey Key);

	UFUNCTION(BlueprintCallable)
	void ExecuteAttack(int32 Index);
	
protected:
	TArray<FName> MeleeEnabledBones;
	FCharacterStats* CharacterStats;

	UPROPERTY()
	TArray<AActor*> RecentlyDamagedActors;
	
	UPROPERTY(EditAnywhere)
	FCharacterBaseStats BaseStats;

	UPROPERTY(EditAnywhere)
	UAnimMontage* GetHitAnimation;

	UPROPERTY(EditAnywhere)
	UAnimMontage* DeathAnimation;

	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnExecuteAttack(const FAttackProperties& Properties);
	UFUNCTION()
	void OnGetHit(const FCustomDamageEvent& DamageEvent);
	UFUNCTION()
	void OnDeath(const FCustomDamageEvent& DamageEvent);
	UFUNCTION()
	void OnMeshOverlapEvent(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp,  int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
