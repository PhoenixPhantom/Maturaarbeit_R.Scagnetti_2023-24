// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/OpponentCharacter.h"

#include "Utility/CombatManager.h"
#include "Characters/Fighters/Opponents/AI/OpponentController.h"
#include "Characters/Fighters/Player/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Utility/NonPlayerFunctionality/TargetInformationComponent.h"
#include "Utility/Savegame/SavableObjectMarkerComponent.h"

AOpponentCharacter::AOpponentCharacter() : bCanBecomeAggressive(true), RequestedAggressionTokens(1),
	AggressionPriority(1.f), AggressionRange(1.f)
{
	SavableObjectMarkerComponent = CreateDefaultSubobject<USavableObjectMarkerComponent>(TEXT("SavableObjectMarkerComp"));
	TargetInformationComponent = CreateDefaultSubobject<UTargetInformationComponent>(TEXT("TargetInformationComp"));

	TargetInformationComponent->SetupAttachment(RootComponent);
}

float AOpponentCharacter::GenerateAggressionScore(APlayerCharacter* PlayerCharacter)
{
	if(!bCanBecomeAggressive) return -1.f;
	float Score = 0.f;
	if(bIsCurrentTarget) Score += 5.f; //TODO: replace arbitrary bonus
	//Aggression priority
	if(AggressionRange > 0.f) Score += std::max(AggressionPriority * (1.f - FVector::Distance(PlayerCharacter->GetActorLocation(),
			GetActorLocation())/AggressionRange), 0.0);
	//Action rank
	Score += PlayerCharacter->RequestActionRank(this);
	return Score;
}

void AOpponentCharacter::BeginPlay()
{
	CharacterStats = new FCharacterStats();
	CharacterStats->FromBase(BaseStats, StatsModifiers, GetWorld());
	Super::BeginPlay();
}

