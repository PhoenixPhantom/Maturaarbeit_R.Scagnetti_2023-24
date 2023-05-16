// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/OpponentCharacter.h"

#include "Characters/Fighters/Opponents/CombatManager.h"
#include "Characters/Fighters/Opponents/OpponentController.h"
#include "Characters/Fighters/Player/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Utility/NonPlayerFunctionality/TargetInformationComponent.h"
#include "Utility/Savegame/SavableObjectMarkerComponent.h"

AOpponentCharacter::AOpponentCharacter()
{
	SavableObjectMarkerComponent = CreateDefaultSubobject<USavableObjectMarkerComponent>(TEXT("SavableObjectMarkerComp"));
	TargetInformationComponent = CreateDefaultSubobject<UTargetInformationComponent>(TEXT("TargetInformationComp"));

	TargetInformationComponent->SetupAttachment(RootComponent);
}

float AOpponentCharacter::GetFieldOfView() const
{
	return CastChecked<AOpponentController>(Controller)->GetFieldOfView();
}

float AOpponentCharacter::GenerateAggressionScore()
{
	if(!bCanBecomeAggressive) return NAN;
	float Score = 0.f;
	if(bIsCurrentTarget) Score += 5.f; //TODO: Make this number be anything that makes sense
	if(!OnGenerateAggressionScore.IsBound())
	{
		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACombatManager::StaticClass(), Actors);
		const ACombatManager* CombatManager = CastChecked<ACombatManager>(Actors[0]);
		OnGenerateAggressionScore.BindDynamic(CombatManager->GetPlayerCharacter(), &APlayerCharacter::OnGenerateAggressionScore);
	}
	Score += OnGenerateAggressionScore.Execute(this);
	return Score;
}

void AOpponentCharacter::BeginPlay()
{
	CharacterStats = new FCharacterStats();
	CharacterStats->FromBase(BaseStats, StatsModifiers, GetWorld());
	Super::BeginPlay();
}

