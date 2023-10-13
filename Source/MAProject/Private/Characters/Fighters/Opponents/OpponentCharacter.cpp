// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/OpponentCharacter.h"

#include <Characters/AdvancedCharacterMovementComponent.h>

#include "Characters/Fighters/Attacks/AttackTree/AttackTreeNode.h"
#include "Characters/Fighters/Player/PlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "UserInterface/HUD/Worldspace/PlayerFacingWidgetComponent.h"
#include "Utility/Animation/SuckToTargetComponent.h"
#include "Utility/Navigation/PatrolManagerComponent.h"
#include "Utility/NonPlayerFunctionality/CharacterRotationManagerComponent.h"
#include "Utility/NonPlayerFunctionality/TargetInformationComponent.h"
#include "Utility/Savegame/SavableObjectMarkerComponent.h"

AOpponentCharacter::AOpponentCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.
		SetDefaultSubobjectClass<UAdvancedCharacterMovementComponent>(ACharacter::CharacterMovementComponentName)),
	bCanBecomeAggressive(true), TargetPlayer(nullptr), RequestedAggressionTokens(1), AggressionPriority(1.f),
	AggressionRange(1.f)
{
	AdvancedCharacterMovementComponent = CastChecked<UAdvancedCharacterMovementComponent>(GetCharacterMovement());
	SavableObjectMarkerComponent = CreateDefaultSubobject<USavableObjectMarkerComponent>(TEXT("SavableObjectMarkerComp"));
	PatrolManagerComponent = CreateDefaultSubobject<UPatrolManagerComponent>(TEXT("PatrolManagerComp"));

	RotationManagerComponent = CreateDefaultSubobject<UCharacterRotationManagerComponent>(TEXT("RotationManagerComp"));
	RotationManagerComponent->SetupAttachment(RootComponent);
	
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

	SetUsePassiveSpace();
}

void AOpponentCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if(EndPlayReason == EEndPlayReason::Destroyed)
	{
		if(IsValid(Controller))	Controller->Destroy();
	}
}

void AOpponentCharacter::BindOnAggressionTokensGranted(const TDelegate<void()>& FunctionToBind, FEditOnAggressionTokensGrantedOrReleasedKey)
{
	OnAggressionTokensGranted = FunctionToBind;
}

void AOpponentCharacter::BindOnAggressionTokensReleased(const TDelegate<void()>& FunctionToBind, FEditOnAggressionTokensGrantedOrReleasedKey)
{
	OnAggressionTokensRemoved = FunctionToBind;
}

void AOpponentCharacter::ExecuteOnAggressionTokensGranted(FExecuteOnAggressionTokensGrantedKey) const
{
	SetUseActiveCombatSpace();
	// ReSharper disable once CppExpressionWithoutSideEffects
	OnAggressionTokensGranted.ExecuteIfBound();
}

void AOpponentCharacter::ExecuteOnAggressionTokensReleased(FExecuteOnAggressionTokensReleasedKey) const
{
	SetUsePassiveSpace();
	// ReSharper disable once CppExpressionWithoutSideEffects
	OnAggressionTokensRemoved.ExecuteIfBound();
}

FRequiredSpace AOpponentCharacter::GetRequiredSpace() const
{
	if(RequiredSpaceActiveCombat->ComponentHasTag(RequiredSpaceActiveTag)) return RequiredSpaceActiveCombat;
	return RequiredSpacePassive;
}

USphereComponent* AOpponentCharacter::GetRequiredSpaceActive() const
{
	return RequiredSpaceActiveCombat;
}

FCircularDistanceConstraint AOpponentCharacter::GetActivePlayerDistanceConstraint() const
{
	FCircularDistanceConstraint DistanceConstraint(TargetPlayer, true);
	if(IsValid(RequestedAttack))
	{
		DistanceConstraint.MaxRadius = FMath::Max(RequestedAttack->GetAttackProperties().MaximalMovementDistance - 100.f, 50.f);
		DistanceConstraint.MinRadius = 0.f;
		DistanceConstraint.OptimalMaxRadius = FMath::Max(RequestedAttack->GetAttackProperties().DefaultMovementDistance - 50.f, 25.f);
		DistanceConstraint.OptimalMinRadius = 0.f;
		return DistanceConstraint;
	}
	bool FoundExecutableAttack = false;
	float TotalDistance = 0.f;
	float MaxDistance = std::numeric_limits<float>::lowest();
	float NumValidAttacks = 0.f;

	const UGenericGraphNode* SourceNode = CharacterStats->Attacks.GetCurrentNode(GetWorld());
	for(const UGenericGraphNode* ChildNode : SourceNode->ChildrenNodes)
	{
		const FAttackProperties& AttackProperties = CastChecked<UAttackTreeNode>(ChildNode)->GetAttackProperties();
		if(!FoundExecutableAttack)
		{
			//the first actually executable attack resets the values because it is stronger than all non-executable ones
			if(!AttackProperties.GetIsOnCd())
			{
				FoundExecutableAttack = true;
				TotalDistance = 0.f;
				MaxDistance = std::numeric_limits<float>::lowest();
				NumValidAttacks = 0.f;
			}
		}
		else if(AttackProperties.GetIsOnCd()) continue;
		
		NumValidAttacks += 1.f;
		if(MaxDistance < AttackProperties.MaximalMovementDistance)
		{
			MaxDistance = AttackProperties.MaximalMovementDistance;
		}
		TotalDistance += AttackProperties.DefaultMovementDistance;
	}
	
	const float DistanceAverage = SourceNode->ChildrenNodes.IsEmpty() ? -1.f :
		TotalDistance/NumValidAttacks;
	
	DistanceConstraint.MaxRadius = FMath::Max(MaxDistance - 100.f, 50.f);
	DistanceConstraint.MinRadius = 0.f;
	DistanceConstraint.OptimalMaxRadius = FMath::Max(DistanceAverage - 50.f, 25.f);
	DistanceConstraint.OptimalMinRadius = 0.f;
	return DistanceConstraint;
}

AController* AOpponentCharacter::GetCombatTargetController() const
{
	if(TargetPlayer == DistanceFromTargetPassive.AnchorController && IsValid(TargetPlayer)) return TargetPlayer;
	return nullptr;
}

ACharacter* AOpponentCharacter::GetCombatTarget() const
{
	if(TargetPlayer == DistanceFromTargetPassive.AnchorController && IsValid(TargetPlayer)) return TargetPlayer->GetCharacter();
	return nullptr;
}

void AOpponentCharacter::RegisterCombatTarget(AController* NewOpponent, FSetCombatTargetKey Key)
{
	if(!IsValid(NewOpponent))
	{
		TargetPlayer = DistanceFromTargetPassive.AnchorController = nullptr;
		RotationManagerComponent->SetRotationMode(ECharacterRotationMode::OrientToMovement, true);
	}
	else
	{
		TargetPlayer = DistanceFromTargetPassive.AnchorController = NewOpponent;
		RotationManagerComponent->SetRotationMode(ECharacterRotationMode::OrientToTarget, true,
			GetTargetPlayer());
	}
}

void AOpponentCharacter::SetUsedBlackboardComponent(UBlackboardComponent* NewBlackboard, FSetUsedBlackboardKey)
{
	UsedBlackboardComponent = NewBlackboard;
}

bool AOpponentCharacter::ExecuteAttackFromNode(UAttackTreeNode* NodeToExecute, FExecuteAttackKey) const
{
	return CharacterStats->Attacks.ExecuteAttackFromNode(NodeToExecute, GetWorld());
}

UAttackTreeNode* AOpponentCharacter::GetRequestedAttack() const
{
	return RequestedAttack;
}

UAttackTreeNode* AOpponentCharacter::GetRandomValidAttack() const
{
	if(!AcceptedInputs.CanOverrideCurrentInput(EInputType::Attack)) return nullptr;

	const ACharacter* TargetCharacter = GetCombatTarget();
	if (!IsValid(TargetCharacter))
	{
		return nullptr;
	}

	const UAttackTreeNode* SourceNode = GetCharacterStats()->Attacks.GetCurrentNode(GetWorld());
	
	//Sort through all available attacks and remove those that cannot be executed
	TArray<TTuple<UGenericGraphNode*, float>> ValidAttacks;
	double TotalScore = 0.f;
	for (UGenericGraphNode* ChildNode : SourceNode->ChildrenNodes)
	{
		const FAttackProperties& AttackProperties = CastChecked<UAttackTreeNode>(ChildNode)->GetAttackProperties();
		if (AttackProperties.GetIsOnCd()) continue;
		const float Priority = AttackProperties.Priority;
		ValidAttacks.Add({ChildNode, Priority});
		TotalScore += Priority;
	}

	if(ValidAttacks.IsEmpty()) return nullptr;

	//Randomly chose a valid attack
	UAttackTreeNode* ChosenNode = nullptr;
	float RandomNumber = FMath::FRandRange(0.0, TotalScore);
	for (const TTuple<UGenericGraphNode*, float>& Attack : ValidAttacks)
	{
		RandomNumber -= Attack.Value;
		if (RandomNumber <= 0.f)
		{
			ChosenNode = CastChecked<UAttackTreeNode>(Attack.Key);
			break;
		}
	}
	return ChosenNode;
}

UAttackTreeNode* AOpponentCharacter::GetRandomValidAttackInRange() const
{
	if(!GetAcceptedInputs().bCanAttack) return nullptr;

	const ACharacter* TargetCharacter = GetCombatTarget();
	if (!IsValid(TargetCharacter))
	{
		return nullptr;
	}

	const float RequiredRange = FVector::Distance(GetTargetPlayer()->GetActorLocation(), GetActorLocation());
	const UAttackTreeNode* SourceNode = GetCharacterStats()->Attacks.GetCurrentNode(GetWorld());
	
	//Sort through all available attacks and remove those that cannot be executed
	TArray<TTuple<UGenericGraphNode*, float>> ValidAttacks;
	double TotalScore = 0.f;
	for (UGenericGraphNode* ChildNode : SourceNode->ChildrenNodes)
	{
		const FAttackProperties& AttackProperties = CastChecked<UAttackTreeNode>(ChildNode)->GetAttackProperties();
		if (AttackProperties.GetIsOnCd() ||
			AttackProperties.MaximalMovementDistance < RequiredRange) continue;
		const float Priority = AttackProperties.Priority;
		ValidAttacks.Add({ChildNode, Priority});
		TotalScore += Priority;
	}

	if(ValidAttacks.IsEmpty()) return nullptr;

	//Randomly chose a valid attack
	UAttackTreeNode* ChosenNode = nullptr;
	float RandomNumber = FMath::FRandRange(0.0, TotalScore);
	for (const TTuple<UGenericGraphNode*, float>& Attack : ValidAttacks)
	{
		RandomNumber -= Attack.Value;
		if (RandomNumber <= 0.f)
		{
			ChosenNode = CastChecked<UAttackTreeNode>(Attack.Key);
			break;
		}
	}
	return ChosenNode;
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

	//Opponents that can't attack right now can become aggressive, but are less likely to
	Score = Score * (1.f - (CanAttackInSeconds() / 5.f));
	return Score;
}

void AOpponentCharacter::BeginPlay()
{
	CharacterStats = new FCharacterStats();
	CharacterStats->FromBase(BaseStats, StatsModifiers, this);
	CharacterStats->Attacks.OnExecuteAttack.AddDynamic(this, &AOpponentCharacter::OnSelectMotionWarpingTarget);

	HealthWidgetComponent->OnHealthMonitorWidgetInitialized.AddDynamic(this, &AOpponentCharacter::RegisterHealthInfoWidget);
	Super::BeginPlay();
}

void AOpponentCharacter::OnDeathTriggered()
{
	Super::OnDeathTriggered();
	bCanBecomeAggressive = false;
}

float AOpponentCharacter::CanAttackInSeconds() const
{
	const float ShortestCd = EarliestAttackSeconds(CharacterStats->Attacks.GetCurrentNode(GetWorld()));
	const float RemainingComboTime = CharacterStats->Attacks.GetLatestAttackProperties().MaxComboTime - GetWorld()->RealTimeSeconds;

	//if the shortest cd remaining is longer than the remaining combo time, the combo will reset and
	//only attacks going from the root node can be accessed
	if(CharacterStats->Attacks.GetCurrentNode(GetWorld()) == CharacterStats->Attacks.GetRootNode() ||
		ShortestCd <= RemainingComboTime) return ShortestCd;
	
	return FMath::Max(RemainingComboTime, EarliestAttackSeconds(CharacterStats->Attacks.GetRootNode()));
}

float AOpponentCharacter::EarliestAttackSeconds(const UGenericGraphNode* SourceNode)
{
	float ShortestCd = std::numeric_limits<float>::max();
	for(const UGenericGraphNode* ChildNode : SourceNode->ChildrenNodes)
	{
		const FAttackProperties& AttackProperties = CastChecked<UAttackTreeNode>(ChildNode)->GetAttackProperties();
		float RemainingCdTime = AttackProperties.CdTimeRemaining();
		if(RemainingCdTime <= 0.f) return 0.f;
		if(RemainingCdTime < ShortestCd)
		{
			ShortestCd = RemainingCdTime;
		}
	}
	return ShortestCd;
}

void AOpponentCharacter::SetUseActiveCombatSpace() const
{
	RequiredSpaceActiveCombat->ComponentTags.AddUnique(RequiredSpaceActiveTag);
	RequiredSpacePassive->ComponentTags.Remove(RequiredSpaceActiveTag);
}

void AOpponentCharacter::SetUsePassiveSpace() const
{
	RequiredSpaceActiveCombat->ComponentTags.Remove(RequiredSpaceActiveTag);
	RequiredSpacePassive->ComponentTags.AddUnique(RequiredSpaceActiveTag);
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

