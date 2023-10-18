// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomDamageEvent.h"
#include "GeneralStats.generated.h"

template<typename Value, typename Percentage>
class TScalable
{
public:
	Value Base;
	Value FlatBonus;
	Percentage PercentageBonus;
	
	TScalable() = default;

	TScalable(const Value& NewBase, const Value& NewFlatBonus, const Percentage& NewPercentageBonus) :
		Base(NewBase), FlatBonus(NewFlatBonus), PercentageBonus(NewPercentageBonus){}
	
	TScalable(const TScalable& Scalable) :
		Base(static_cast<Value>(Scalable.Base)), FlatBonus(static_cast<Value>(Scalable.FlatBonus)),
		PercentageBonus(static_cast<Percentage>(Scalable.PercentageBonus))
	{}

	Value GetResulting() const
	{
		return Base * (1.0 + PercentageBonus/100.0) + FlatBonus;
	};

	Value operator+(const Value& OtherFlatBonus){ return GetResulting() * OtherFlatBonus; }
	Value operator*(const Percentage& OtherScalar){ return GetResulting() * OtherScalar; }
	
	bool operator==(const TScalable& Scalable) const
	{
		return Base == static_cast<Value>(Scalable.Base) &&
			FlatBonus == static_cast<Value>(Scalable.Base) &&
				PercentageBonus == static_cast<Percentage>(Scalable.PercentageBonus);
	}
};

USTRUCT()
struct MAPROJECT_API FSavableModifiersBase
{
	GENERATED_BODY()
	
	FSavableModifiersBase() : Level(1){}

	UPROPERTY(SaveGame)
	uint32 Level;
};

//This is a helper struct used in tandem with
//FGeneralObjectStats to allow base settings for stats to be tuned in-editor
USTRUCT()
struct MAPROJECT_API FGeneralBaseStats
{
	GENERATED_BODY()
	
	FGeneralBaseStats() : BaseHealth(100), BaseAttack(100), BaseDefense(100){}

	FGeneralBaseStats(const FGeneralBaseStats& Source) : BaseHealth(Source.BaseHealth), BaseAttack(Source.BaseAttack),
		BaseDefense(Source.BaseDefense){};

	FGeneralBaseStats(const float NewHealth, const uint32 NewBaseDamage = 100, const uint32 NewBaseDefense = 100) :
		BaseHealth(NewHealth), BaseAttack(NewBaseDamage), BaseDefense(NewBaseDefense){};

	bool operator==(const FGeneralBaseStats &GeneralBaseStats) const;

	UPROPERTY(EditDefaultsOnly, meta=(ForceUnits="HP"))
	int32 BaseHealth;

	UPROPERTY(EditDefaultsOnly)
	uint32 BaseAttack;

	UPROPERTY(EditDefaultsOnly)
	uint32 BaseDefense;	
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedDelegate, uint32, NewHelath, uint32, OldHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGetDamagedDelegate,  const FCustomDamageEvent&, DamageEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNoHealthReachedDelegate, const FCustomDamageEvent&, DamageEvent);

#define USE_UE5_DELEGATE false

struct FGeneralObjectStats
{
	FGeneralObjectStats();
	virtual ~FGeneralObjectStats() = default;

	FOnHealthChangedDelegate OnHealthChanged;

#if USE_UE5_DELEGATE
	FOnGetDamagedDelegate OnGetDamaged;
#else
	//TODO: This is a hack to make passing FCustomDamageEvent* through without loss or errors
	TDelegate<void(const FCustomDamageEvent*)> OnGetDamaged;
#endif
	FOnNoHealthReachedDelegate OnNoHealthReached;

	//Represents the current health of the object. Should not be changed directly but through ReceiveDamage
	uint32 Health;
	TScalable<uint32, float> MaxHealth;

	TScalable<int32, float> Attack;
	TScalable<int32, float> Defense;


	bool operator==(const FGeneralObjectStats& GeneralObjectStats) const;

	void FromBase(const FGeneralBaseStats& Stats, const FSavableModifiersBase& Modifiers);

	virtual float GetDamageOutput() const;
	virtual void GenerateDamageEvent(FCustomDamageEvent& DamageEvent, const FHitResult& HitResult = FHitResult()) const = 0;
	uint32 ReceiveDamage(float Damage, const FCustomDamageEvent* DamageInfo);
};
