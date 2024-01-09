// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/Sound/GlobalSoundManager.h"

#include "Components/ShapeComponent.h"
#include "Engine/TriggerBox.h"

AGlobalSoundManager::AGlobalSoundManager(): Soundscape(nullptr), PlayerController(nullptr), ActiveVolume(nullptr)
{
	PrimaryActorTick.bCanEverTick = false;
}

void AGlobalSoundManager::EnterCombatState(FSetCombatStateKey) const
{
	Soundscape->SetState(CombatTag);
}

void AGlobalSoundManager::EndCombatState(FSetCombatStateKey) const
{
	Soundscape->ClearState(CombatTag);
}

void AGlobalSoundManager::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	for(TTuple<ATriggerBase*, FGameplayTag>& AreaTag : WorldAreaTags)
	{
		const FColor* TargetColor = TagColors.Find(AreaTag.Value);
		if(TargetColor == nullptr || AreaTag.Key == nullptr) continue;
		AreaTag.Key->GetCollisionComponent()->ShapeColor = *TargetColor;
	}
}

void AGlobalSoundManager::BeginPlay()
{
	Super::BeginPlay();
	PlayerController = GetWorld()->GetFirstPlayerController();
	Soundscape = GetWorld()->GetGameInstance()->GetSubsystem<USoundscapeSubsystem>();
	if(!IsValid(Soundscape))
	{
		checkNoEntry();
		return;
	}
	Soundscape->AddPaletteCollection("WorldSounds", RelevantPalettes);
	bool IsInBoundingBox = false;
	for(const TTuple<ATriggerBase*, FGameplayTag>& AreaTag : WorldAreaTags)
	{
		AreaTag.Key->OnActorBeginOverlap.AddDynamic(this, &AGlobalSoundManager::OnBoundingVolumeOverlap);
		if(AreaTag.Key->IsOverlappingActor(PlayerController->GetPawn()))
		{
			IsInBoundingBox = true;
			ActiveVolume = AreaTag.Key;
			Soundscape->SetState(AreaTag.Value);
		}
	}
	if(!IsInBoundingBox)
	{
		//this seems to be the easiest way to just get any element of the map
		for(const TTuple<ATriggerBase*, FGameplayTag>& AreaTag : WorldAreaTags)
		{
			ActiveVolume = AreaTag.Key;
			Soundscape->SetState(AreaTag.Value);
			break;
		}
	}
	
}

void AGlobalSoundManager::OnBoundingVolumeOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if(OtherActor != PlayerController->GetPawn()) return;
	ATriggerBase* OverlappedBox = CastChecked<ATriggerBase>(OverlappedActor);
	FGameplayTag NewTag = WorldAreaTags.FindChecked(OverlappedBox);
	if(!NewTag.IsValid()) return;
	Soundscape->SetState(WorldAreaTags.FindChecked(OverlappedBox));
	FGameplayTag OldTag = WorldAreaTags.FindChecked(ActiveVolume);
	if(NewTag != OldTag) Soundscape->ClearState(OldTag);
	ActiveVolume = OverlappedBox;
}

