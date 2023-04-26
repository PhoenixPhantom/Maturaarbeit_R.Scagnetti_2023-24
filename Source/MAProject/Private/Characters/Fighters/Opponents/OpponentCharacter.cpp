// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/OpponentCharacter.h"

#include "Utility/Savegame/SavableObjectMarkerComponent.h"

AOpponentCharacter::AOpponentCharacter()
{
	SavableObjectMarkerComponent = CreateDefaultSubobject<USavableObjectMarkerComponent>(TEXT("SavableObjectMarkerComp"));
}
