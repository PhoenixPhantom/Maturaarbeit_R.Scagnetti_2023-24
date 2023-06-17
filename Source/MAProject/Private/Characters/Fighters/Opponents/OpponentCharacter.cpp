// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/OpponentCharacter.h"

#include "Utility/CombatManager.h"
#include "Characters/Fighters/Player/PlayerCharacter.h"
#include "Utility/Animation/SuckToTargetComponent.h"
#include "Utility/NonPlayerFunctionality/TargetInformationComponent.h"
#include "Utility/Savegame/SavableObjectMarkerComponent.h"

AOpponentCharacter::AOpponentCharacter() : bCanBecomeAggressive(true), TargetPlayer(nullptr),
	RequestedAggressionTokens(1), AggressionPriority(1.f), AggressionRange(1.f)
{
	SavableObjectMarkerComponent = CreateDefaultSubobject<USavableObjectMarkerComponent>(TEXT("SavableObjectMarkerComp"));
}

void AOpponentCharacter::RegisterPlayerOpponent(AController* NewOpponent, FSetPlayerOpponentKey Key)
{
	if(!IsValid(NewOpponent))
	{
		TargetPlayer = PassiveCombatConstraint.OrientationCenter = DistanceFromTargetActive.Player =
			DistanceFromTargetPassive.Player = nullptr;
	}
	else
	{
		TargetPlayer = PassiveCombatConstraint.OrientationCenter = DistanceFromTargetActive.Player =
			DistanceFromTargetPassive.Player = NewOpponent;
	}
}

float AOpponentCharacter::GenerateAggressionScore(APlayerCharacter* PlayerCharacter) const
{
	if(!bCanBecomeAggressive) return -1.f;
	float Score = 0.f;
	if(TargetInformationComponent->GetIsCurrentTarget()) Score += 1.5f;
	//Aggression priority
	if(AggressionRange > 0.f) Score += AggressionPriority * (1.f - std::min(FVector::Distance(PlayerCharacter->GetActorLocation(),
			GetActorLocation())/AggressionRange, 1.0));
	//Action rank
	Score += PlayerCharacter->RequestActionRank(this);
	return Score;
}

void AOpponentCharacter::BeginPlay()
{
	CharacterStats = new FCharacterStats();
	CharacterStats->FromBase(BaseStats, StatsModifiers, GetWorld());
	ActiveCombatConstraint.Owner = this;
	PassiveCombatConstraint.Owner = this;
	DistanceFromTargetActive.Owner = this;
	DistanceFromTargetPassive.Owner = this;
	CharacterStats->OnExecuteAttack.AddDynamic(this, &AOpponentCharacter::OnSelectMotionWarpingTarget);
	Super::BeginPlay();
}

void AOpponentCharacter::OnSelectMotionWarpingTarget(const FAttackProperties& Properties)
{
	if(IsValid(TargetPlayer)){
		UActorComponent* Component = TargetPlayer->GetPawn()->GetComponentByClass(UTargetInformationComponent::StaticClass());
		if(IsValid(Component))
		{
			UTargetInformationComponent* TargetInfoComp = CastChecked<UTargetInformationComponent>(Component);
			FVector Direction;
			float Length;
			(TargetInfoComp->GetComponentLocation() - GetActorLocation()).ToDirectionAndLength(Direction, Length);
			if(Length <= Properties.MaximalMovementDistance)
				SuckToTargetComponent->SetWarpTargetFaceTowards(TargetInfoComp);
			else
			{
				SuckToTargetComponent->SetWarpTargetFaceTowards(
					GetActorLocation() + Direction * Properties.DefaultMovementDistance, GetActorLocation());
			}
			return;
		}
		checkNoEntry();
	}
	FVector PlayerLocation;
	FRotator PlayerRotation;
	GetActorEyesViewPoint(PlayerLocation, PlayerRotation);
	FVector CharacterForward = PlayerRotation.Vector();
	CharacterForward.Z = 0.f;
	CharacterForward.Normalize();
	SuckToTargetComponent->SetWarpTargetFaceTowards(GetActorLocation() +
		CharacterForward * Properties.DefaultMovementDistance, GetActorLocation());
}

