// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/Stats/StatusEffect.h"

#include "Characters/GeneralCharacter.h"
#include "Components/Image.h"

UStatusEffect::UStatusEffect() : BoundImage(nullptr), EffectTarget(nullptr), Thumbnail(nullptr), MaxEffectTime(-1.f)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UStatusEffect::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	const float TimeRemaining = GetOwner()->GetWorld()->GetTimerManager().GetTimerRemaining(EffectResetHandle);
	if(TimeRemaining <= 0) return;
	BoundImage->GetDynamicMaterial()->SetScalarParameterValue("Alpha", TimeRemaining/MaxEffectTime);
}

void UStatusEffect::ForceRestartTimer(FForceStatusEffectTimerRestartKey)
{
	GetOwner()->GetWorld()->GetTimerManager().SetTimer(EffectResetHandle, this, &UStatusEffect::OnEffectTimeExceeded,
		MaxEffectTime);
}

void UStatusEffect::BindImage(UImage* Image, FStatusEffectBindImageKey)
{
	if(!IsValid(Image)) return;
	if(IsValid(BoundImage))
	{
		BoundImage->SetVisibility(ESlateVisibility::Hidden);
	}
	BoundImage = Image;
	if(IsValid(Thumbnail)) BoundImage->SetBrushResourceObject(Thumbnail);
	BoundImage->SetVisibility(ESlateVisibility::Visible);
	PrimaryComponentTick.SetTickFunctionEnable(true);
}

void UStatusEffect::OnEffectApplied_Implementation(AGeneralCharacter* Target)
{
	OnEffectApplied(Target);
	EffectTarget = Target;
	GetWorld()->GetTimerManager().SetTimer(EffectResetHandle, this, &UStatusEffect::OnEffectTimeExceeded,
		MaxEffectTime);
}

void UStatusEffect::OnEffectRemoved_Implementation(AGeneralCharacter* Target)
{
	OnEffectRemoved(Target);
	if(IsValid(BoundImage))	BoundImage->SetVisibility(ESlateVisibility::Hidden);
}


void UStatusEffect::OnEffectTimedOut_Implementation(AGeneralCharacter* Target)
{
	OnEffectTimedOut(Target);
}

void UStatusEffect::OnEffectTimeExceeded()
{
	OnEffectTimedOut_Implementation(EffectTarget);
	EffectTarget->RemoveStatusEffectExternal(this, FModifyCharacterStatusEffectKey());
}