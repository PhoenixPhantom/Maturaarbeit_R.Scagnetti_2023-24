// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatusEffect.generated.h"

class APlayerCharacter;
class UImage;
class AGeneralCharacter;

struct FForceStatusEffectTimerRestartKey final
{
	friend AGeneralCharacter;
private:
	FForceStatusEffectTimerRestartKey(){}
};
struct FStatusEffectBindImageKey final
{
	friend APlayerCharacter;
private:
	FStatusEffectBindImageKey(){}
};

/** Base class of all status effects
 * Defines how status effects are handled in the most
 * general sense
 */
UCLASS(Blueprintable, BlueprintType)
class MAPROJECT_API UStatusEffect : public UActorComponent
{
	GENERATED_BODY()
public:
	UStatusEffect();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FTimerHandle GetTimerHandle() const { return EffectResetHandle; }
	float GetMaxEffectTime() const { return MaxEffectTime; }
	void ForceRestartTimer(FForceStatusEffectTimerRestartKey);
	void BindImage(UImage* Image, FStatusEffectBindImageKey);
	bool IsBoundImage(const UImage* Image) const { return BoundImage == Image; }

	void OnEffectApplied_Implementation(AGeneralCharacter* Target);
	void OnEffectRemoved_Implementation(AGeneralCharacter* Target);
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnEffectApplied(AGeneralCharacter* Target);
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnEffectRemoved(AGeneralCharacter* Target);

protected:
	FTimerHandle EffectResetHandle;

	UPROPERTY()
	UImage* BoundImage; 
	
	UPROPERTY()
	AGeneralCharacter* EffectTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Brush, meta=( AllowPrivateAccess="true",
			DisplayThumbnail="true", DisplayName="Image",
			AllowedClasses="/Script/Engine.Texture,/Script/Engine.MaterialInterface,/Script/Engine.SlateTextureAtlasInterface",
			DisallowedClasses = "/Script/MediaAssets.MediaTexture"))
	UObject* Thumbnail;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxEffectTime;

	void OnEffectTimedOut_Implementation(AGeneralCharacter* Target);

	UFUNCTION()
	void OnEffectTimeExceeded();
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnEffectTimedOut(AGeneralCharacter* Target);	
};
