// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/OpponentCharacter.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Utility/Savegame/SavableObjectMarkerComponent.h"

AOpponentCharacter::AOpponentCharacter() : ControllingBlackboard(nullptr)
{
	SavableObjectMarkerComponent = CreateDefaultSubobject<USavableObjectMarkerComponent>(TEXT("SavableObjectMarkerComp"));
	PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));
	PawnSensingComponent->OnSeePawn.AddDynamic(this, &AOpponentCharacter::OnSeePawn);
}

float AOpponentCharacter::GetFieldOfView() const
{
	return PawnSensingComponent->GetPeripheralVisionAngle()*2.f;
}


void AOpponentCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	AAIController* AIController = Cast<AAIController>(Controller);
	ControllingBlackboard = AIController->GetBlackboardComponent();
}

void AOpponentCharacter::UnPossessed()
{
	ControllingBlackboard = nullptr;
	Super::UnPossessed();
}

void AOpponentCharacter::OnSeePawn(APawn* Pawn)
{
	if(!IsValid(ControllingBlackboard)) return;
	ControllingBlackboard->SetValueAsBool("HasSensedPlayer", true);
	ControllingBlackboard->SetValueAsVector("TargetLocation", Pawn->GetActorLocation());
}

