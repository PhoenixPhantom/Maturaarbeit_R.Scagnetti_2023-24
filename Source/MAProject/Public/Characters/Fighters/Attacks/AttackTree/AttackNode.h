// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttackTreeBaseNode.h"
#include "Characters/Fighters/Attacks/AttackProperties.h"
#include "AttackNode.generated.h"

class UStatusEffect;
class UCustomAnimInstance;

USTRUCT()
struct FAttackPropertiesNode : public FAttackProperties
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	FString AttackTitle;
	
	FAttackPropertiesNode();
	
	FAttackPropertiesNode(const FAttackPropertiesNode& Properties);
	FAttackPropertiesNode(const FAttackProperties& Properties);
	
	bool operator==(const FAttackPropertiesNode& AttackProperties) const;
};

USTRUCT()
struct FAttackPropertiesNodeAdditional : public FAttackPropertiesNode
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UStatusEffect> ExecutionCondition;
	
	FAttackPropertiesNodeAdditional();
	
	FAttackPropertiesNodeAdditional(const FAttackPropertiesNodeAdditional& Properties);
	FAttackPropertiesNodeAdditional(const FAttackPropertiesNode& Properties);
	FAttackPropertiesNodeAdditional(const FAttackProperties& Properties);
	
	bool operator==(const FAttackPropertiesNode& AttackProperties) const;
};
/**
 * 
 */
UCLASS(Blueprintable)
class MAPROJECT_API UAttackNode : public UAttackTreeBaseNode
{
	GENERATED_BODY()
public:
	UAttackNode();

	virtual const FAttackProperties& GetAttackProperties(const AActor* PlayingInstance = nullptr) const;
	virtual bool CanBeCalled(const FString& NodeName) const;
	bool GetIsOnCd() const { return bIsOnCd; }
	FTimerHandle GetTimerHandle() const{ return CdHandle; }
	void Execute();
	void ForceSetCd(float DesiredCd);
	float CdTimeElapsed() const;
	float CdTimeRemaining() const;

#if WITH_EDITOR
	virtual FText GetNodeTitle() const override;
	virtual void SetNodeTitle(const FText& NewTitle) override;
	virtual FLinearColor GetBackgroundColor() const override;
#endif
protected:
	bool bIsOnCd;
	FTimerHandle CdHandle;

	UPROPERTY(EditDefaultsOnly, Category = AttackProperties)
	FAttackPropertiesNode AttackProperties;
	UPROPERTY(EditDefaultsOnly, Category = AttackProperties)
	TArray<FAttackPropertiesNodeAdditional> AdditionalAttacks;
};
