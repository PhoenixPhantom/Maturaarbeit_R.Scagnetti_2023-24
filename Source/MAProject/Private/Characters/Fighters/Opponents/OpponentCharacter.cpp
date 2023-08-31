// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/OpponentCharacter.h"

#include "Characters/Fighters/Player/PlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "UserInterface/HUD/Worldspace/PlayerFacingWidgetComponent.h"
#include "Utility/Animation/SuckToTargetComponent.h"
#include "Utility/NonPlayerFunctionality/TargetInformationComponent.h"
#include "Utility/Savegame/SavableObjectMarkerComponent.h"

AOpponentCharacter::AOpponentCharacter() : bCanBecomeAggressive(true), TargetPlayer(nullptr),
	RequestedAggressionTokens(1), AggressionPriority(1.f), AggressionRange(1.f)
{	
	SavableObjectMarkerComponent = CreateDefaultSubobject<USavableObjectMarkerComponent>(TEXT("SavableObjectMarkerComp"));
	
	RequiredSpaceActiveCombat = CreateDefaultSubobject<USphereComponent>(TEXT("ActiveCombatSize"));
	RequiredSpaceActiveCombat->SetupAttachment(GetMesh());
	RequiredSpaceActiveCombat->SetCollisionProfileName("PawnScanner");
	RequiredSpaceActiveCombat->SetGenerateOverlapEvents(true);
	RequiredSpaceActiveCombat->SetSphereRadius(160.f);
	RequiredSpaceActiveCombat->SetCanEverAffectNavigation(false);
	
	RequiredSpacePassive = CreateDefaultSubobject<UBoxComponent>(TEXT("PassiveCombatSize"));
	RequiredSpacePassive->SetupAttachment(GetMesh());
	RequiredSpacePassive->SetCollisionProfileName("PawnScanner");
	RequiredSpacePassive->SetGenerateOverlapEvents(true);
	RequiredSpacePassive->SetBoxExtent(FVector(50.f, 50.f, 100.f));
	RequiredSpacePassive->SetCanEverAffectNavigation(false);
	
	HealthWidgetComponent = CreateDefaultSubobject<UPlayerFacingWidgetComponent>(TEXT("HealthWidgetComp"));
	HealthWidgetComponent->SetupAttachment(RootComponent);
	HealthWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
}

void AOpponentCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if(EndPlayReason == EEndPlayReason::Destroyed)
	{
		if(IsValid(Controller))	Controller->Destroy();
	}
}

UShapeComponent* AOpponentCharacter::GetRequiredSpace() const
{
	if(RequiredSpaceActiveCombat->ComponentHasTag(RequiredSpaceActiveTag)) return RequiredSpaceActiveCombat;
	return RequiredSpacePassive;
}

FCircularDistanceConstraint AOpponentCharacter::GetActivePlayerDistanceConstraint() const
{
	FCircularDistanceConstraint DistanceConstraint(TargetPlayer);
	float TotalDistance = 0.f;
	float MaxDistance = std::numeric_limits<float>::lowest();
	float NumValidAttacks = 0.f;
	for(const FAttackProperties& AttackProperties : CharacterStats->AvailableAttacks)
	{
		if(AttackProperties.GetIsOnCd()) continue;
		NumValidAttacks += 1.f;
		if(MaxDistance < AttackProperties.MaximalMovementDistance)
		{
			MaxDistance = AttackProperties.MaximalMovementDistance;
		}
		TotalDistance += AttackProperties.DefaultMovementDistance;
	}
	const float DistanceAverage = CharacterStats->AvailableAttacks.IsEmpty() ? -1.f :
		TotalDistance/NumValidAttacks;
	
	DistanceConstraint.MaxRadius = MaxDistance*0.9f;
	DistanceConstraint.MinRadius = 0.f;
	DistanceConstraint.OptimalMaxRadius = DistanceAverage*0.9f;
	DistanceConstraint.OptimalMinRadius = 0.f;
	return DistanceConstraint;
}

AController* AOpponentCharacter::GetRegisteredPlayerOpponent() const
{
	if(TargetPlayer == DistanceFromTargetPassive.AnchorController) return TargetPlayer;
	return nullptr;
}

void AOpponentCharacter::RegisterPlayerOpponent(AController* NewOpponent, FSetPlayerOpponentKey Key)
{
	if(!IsValid(NewOpponent))
	{
		TargetPlayer = DistanceFromTargetPassive.AnchorController = nullptr;
	}
	else
	{
		TargetPlayer = DistanceFromTargetPassive.AnchorController = NewOpponent;
	}
}

float AOpponentCharacter::GenerateAggressionScore(APlayerCharacter* PlayerCharacter) const
{
	if(!bCanBecomeAggressive) return -1.f;
	if(!CanAttack()) return 0.f;
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
	CharacterStats->OnExecuteAttack.AddDynamic(this, &AOpponentCharacter::OnSelectMotionWarpingTarget);
	OnAggressionTokensGranted.AddDynamic(this, &AOpponentCharacter::SetUseActiveCombatSpace);
	OnAggressionTokensRemoved.AddDynamic(this, &AOpponentCharacter::SetUsePassiveSpace);
	SetUsePassiveSpace();

	HealthWidgetComponent->OnHealthMonitorWidgetInitialized.AddDynamic(this, &AOpponentCharacter::RegisterHealthInfoWidget);
	Super::BeginPlay();
}

void AOpponentCharacter::OnDeathTriggered()
{
	Super::OnDeathTriggered();
	bCanBecomeAggressive = false;
}

bool AOpponentCharacter::CanAttack() const
{
	for(const FAttackProperties& AttackProperties : CharacterStats->AvailableAttacks)
	{
		//Only if there is any attack to be executed, the opponent can become aggressive
		if(!AttackProperties.GetIsOnCd()) return true;
	}
	return false;
}

void AOpponentCharacter::SetUseActiveCombatSpace()
{
	
	RequiredSpaceActiveCombat->ComponentTags.AddUnique(RequiredSpaceActiveTag);
	RequiredSpacePassive->ComponentTags.Remove(RequiredSpaceActiveTag);
}

void AOpponentCharacter::SetUsePassiveSpace()
{
	RequiredSpaceActiveCombat->ComponentTags.AddUnique(RequiredSpaceActiveTag);
	RequiredSpacePassive->ComponentTags.Remove(RequiredSpaceActiveTag);
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

			FWarpInformation WarpInformation;
			if (Length <= Properties.MaximalMovementDistance)
			{
				WarpInformation.WarpSource = EWarpSource::FaceTargetObject;
				WarpInformation.TargetObject = TargetInfoComp;
				WarpInformation.bFollowTarget = true;
				WarpInformation.MaxWarpingDistance = Properties.MaximalMovementDistance;
			}
			else
			{
				WarpInformation.WarpSource = EWarpSource::FaceLocation;
				WarpInformation.TargetLocation = GetActorLocation() + Direction * Properties.DefaultMovementDistance;
			}
			SuckToTargetComponent->SetOrUpdateWarpTarget(WarpInformation);
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
	FWarpInformation WarpInformation;
	WarpInformation.WarpSource = EWarpSource::FaceLocation;
	WarpInformation.TargetLocation = GetActorLocation() + CharacterForward * Properties.DefaultMovementDistance;
	SuckToTargetComponent->SetOrUpdateWarpTarget(WarpInformation);
}

