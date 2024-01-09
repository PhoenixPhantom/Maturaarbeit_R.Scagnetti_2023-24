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
		return (PercentageBonus < 0 ? 1 : Base * (1.0 + PercentageBonus/100.0)) + FlatBonus;
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

template<typename Value, typename Percentage>
class TMaxedValue{
public:
	typedef TScalable<Value, Percentage> FMaxVal;
	
	FMaxVal Maximum;
	Value Current;
	
	TMaxedValue() : Maximum(0, 0, 0), Current(0){}
	TMaxedValue(const Value& NewBase, const Value& NewFlatBonus, const Percentage& NewPercentageBonus) :
		Maximum(NewBase, NewFlatBonus, NewPercentageBonus){ Current = Maximum.GetResulting(); }
	TMaxedValue(const FMaxVal& Scalable) : Maximum(Scalable){ Current = Maximum.GetResulting(); }
	TMaxedValue(const TMaxedValue& MaxedValue) : Maximum(MaxedValue), Current(MaxedValue.Current){}

	void SetBaseMax(const Value& NewBase)
	{
		Maximum.Base = NewBase;
		Current = Maximum.GetResulting();
	}
	void Reset()
	{
		Maximum.FlatBonus = 0.f;
		Maximum.PercentageBonus = 0.f;
		Current = Maximum.GetResulting();
	}
	void AddBonuses(const Value& FlatBonus, const Percentage& PercentageBonus)
	{
		const double CurrentRatio = static_cast<double>(Current)/static_cast<double>(Maximum.GetResulting());
		Maximum.FlatBonus += FlatBonus;
		Maximum.FlatBonus += PercentageBonus;
		const double NewCurrent = round(CurrentRatio * static_cast<double>(Maximum.GetResulting()));
		Current = (Current != 0 && NewCurrent == 0.0 ? 1.0 : NewCurrent);
	}
	
	bool operator==(const TMaxedValue& Scalable) const
	{
		return Maximum == Scalable.Maximum && Current == static_cast<Value>(Scalable.Current);
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
		BaseDefense(Source.BaseDefense){}

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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedDelegate, int32, New, int32, Old);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMaxHealthChangedDelegate, int32, NewCurrent, int32, NewMax);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMinHealthReachedDelegate);


USTRUCT(BlueprintType)
struct FGeneralObjectStatsBuffs
{
	GENERATED_BODY()
public:
	virtual ~FGeneralObjectStatsBuffs() = default;

	FGeneralObjectStatsBuffs() : HealthBuff(0), AttackBuff(0), DefenseBuff(0), FlatHealth(0), FlatAttack(0),
	                             FlatDefense(0){}

	FGeneralObjectStatsBuffs(float HB, float AB, float DB, float FH, float FA, float FD);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Units="%"))
	float HealthBuff;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Units="%"))
	float AttackBuff;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Units="%"))
	float DefenseBuff;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ForceUnits="HP"))
	float FlatHealth;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FlatAttack;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FlatDefense;

	FGeneralObjectStatsBuffs ReverseGeneralObjectBuffs() const;
};


struct FGeneralObjectStats
{
	FGeneralObjectStats();
	virtual ~FGeneralObjectStats() = default;

	FOnHealthChangedDelegate OnHealthChanged;
	FOnMaxHealthChangedDelegate OnMaxHealthChanged;

	//This is a hack to make passing FCustomDamageEvent* through without loss or errors
	TDelegate<void(const FCustomDamageEvent*)> OnGetDamaged;
	
	FOnMinHealthReachedDelegate OnNoHealthReached;

	//Represents the current health of the object. Should not be changed directly but through ReceiveDamage
	TMaxedValue<int32, float> Health;

	TScalable<int32, float> Attack;
	TScalable<int32, float> Defense;


	bool operator==(const FGeneralObjectStats& GeneralObjectStats) const;

	void FromBase(const FGeneralBaseStats& Stats, const FSavableModifiersBase& Modifiers);
	virtual void Reset();
	void ResetHealth();
	void ResetAttack();
	void ResetDefense();
	void Buff(const FGeneralObjectStatsBuffs& Buffs);
	void Debuff(const FGeneralObjectStatsBuffs& Buffs);

	virtual float GetDamageOutput() const;
	virtual void GenerateDamageEvent(FCustomDamageEvent& DamageEvent, const FHitResult& HitResult = FHitResult()) const = 0;
	int32 ReceiveDamage(float Damage, const FCustomDamageEvent* DamageInfo);
	int32 ReceiveDamage(float Damage){ return ChangeHealth(-Damage/static_cast<float>(Defense.GetResulting())); }
	int32 ChangeHealth(int32 DeltaHealth);
	FORCEINLINE int32 ChangeHealthByPercentage(float Percentage)
	{ 
		return ChangeHealth(static_cast<float>(Health.Maximum.GetResulting()) * Percentage/100.f);
	}
};
