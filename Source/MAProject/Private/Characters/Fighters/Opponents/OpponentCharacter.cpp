// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Opponents/OpponentCharacter.h"

#include <Characters/AdvancedCharacterMovementComponent.h>

#include "Characters/Fighters/Attacks/AttackTree/AttackNode.h"
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
	FCircularDistanceConstraint DistanceConstraint(TargetPlayer);
	if(IsValid(RequestedAttack))
	{
		//the max values are decreased slightly as the move to request has a leniency radius that ends the movement
		//before actually reaching the target point. This is required to prevent the character from permanently getting stuck
		//but also means that attacks can sometimes not be executed due to being out of range even though the character shouldn't.
		DistanceConstraint.MaxRadius = FMath::Max(RequestedAttack->GetAttackProperties().MaximalMovementDistance - 100.f, 50.f);
		DistanceConstraint.MinRadius = RequestedAttack->GetAttackProperties().MinimalMovementDistance;
		DistanceConstraint.OptimalMaxRadius = FMath::Max(RequestedAttack->GetAttackProperties().DefaultMovementDistance - 50.f, 25.f);
		DistanceConstraint.OptimalMinRadius = RequestedAttack->GetAttackProperties().MinimalMovementDistance;
		DistanceConstraint.bUseNavPath = RequestedAttack->GetAttackProperties().bIsMeleeAttack;
		return DistanceConstraint;
	}
	bool FoundExecutableAttack = false;
	float TotalDistance = 0.f;
	float MaxDistance = std::numeric_limits<float>::lowest();
	float MinDistance = std::numeric_limits<float>::max();
	bool MaxRequiresNavPath = false;
	bool MinRequiresNavPath = false;
	float NumValidAttacks = 0.f;

	//if there is no requested attack, we return the average of the most likely to execute attacks (those not on cooldown and otherwise all)
	const UGenericGraphNode* SourceNode = CharacterStats->Attacks.GetCurrentNode(GetWorld());
	for(const UGenericGraphNode* ChildNode : SourceNode->ChildrenNodes)
	{
		const UAttackNode* AttackNode = CastChecked<UAttackNode>(ChildNode);
		const FAttackProperties& AttackProperties = AttackNode->GetAttackProperties();
		if(!FoundExecutableAttack)
		{
			//the first actually executable attack resets the values because it is stronger than all non-executable ones
			if(!AttackNode->GetIsOnCd())
			{
				FoundExecutableAttack = true;
				TotalDistance = 0.f;
				MaxDistance = std::numeric_limits<float>::lowest();
				MinDistance = std::numeric_limits<float>::max();
				MaxRequiresNavPath = false;
				MinRequiresNavPath = false;
				NumValidAttacks = 0.f;
			}
		}
		else if(AttackNode->GetIsOnCd()) continue;
		
		NumValidAttacks += 1.f;
		if(MaxDistance < AttackProperties.MaximalMovementDistance)
		{
			MaxRequiresNavPath = AttackProperties.bIsMeleeAttack;
			MaxDistance = AttackProperties.MaximalMovementDistance;
		}
		if(MinDistance > AttackProperties.MinimalMovementDistance)
		{
			MinRequiresNavPath = AttackProperties.bIsMeleeAttack;
			MinDistance = AttackProperties.MinimalMovementDistance;
		}
		TotalDistance += AttackProperties.DefaultMovementDistance;
	}
	
	const float DistanceAverage = SourceNode->ChildrenNodes.IsEmpty() ? -1.f :
		TotalDistance/NumValidAttacks;

	
	//The max values are decreased slightly as the move to request has a lineancy radius that ends the movement
	//before actually reaching the target point. This is required to prevent the character from permanently getting stuck
	//but also means that attacks can sometimes not be executed due to being out of range even though the character shouldn't.
	DistanceConstraint.MaxRadius = FMath::Max(MaxDistance - 100.f, 50.f);
	DistanceConstraint.MinRadius = MinDistance;
	DistanceConstraint.OptimalMaxRadius = FMath::Max(DistanceAverage - 50.f, 25.f);
	DistanceConstraint.OptimalMinRadius = MinDistance;
	DistanceConstraint.bUseNavPath = MaxRequiresNavPath || MinRequiresNavPath;
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

void AOpponentCharacter::ClearCombatTarget(FClearCombatTargetKey)
{
	TargetPlayer = DistanceFromTargetPassive.AnchorController = nullptr;
	RotationManagerComponent->SetRotationMode(ECharacterRotationMode::OrientToMovement, true);
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
			TargetPlayer->GetPawn());
	}
}

void AOpponentCharacter::SetUsedBlackboardComponent(UBlackboardComponent* NewBlackboard, FSetUsedBlackboardKey)
{
	UsedBlackboardComponent = NewBlackboard;
}

bool AOpponentCharacter::ExecuteAttackFromNode(UAttackNode* NodeToExecute, FExecuteAttackKey) const
{
	return CharacterStats->Attacks.ExecuteAttackFromNode(NodeToExecute, this, GetWorld());
}

UAttackNode* AOpponentCharacter::GetRequestedAttack() const
{
	return RequestedAttack;
}

UAttackNode* AOpponentCharacter::GetRandomValidAttack() const
{
	if(!AcceptedInputs.IsAllowedInput(EInputType::Attack)) return nullptr;

	if (GetCombatTarget() == nullptr)
	{
		return nullptr;
	}

	const UGenericGraphNode* SourceNode = GetCharacterStats()->Attacks.GetCurrentNode(GetWorld());
	
	//Sort through all available attacks and remove those that cannot be executed
	TArray<TTuple<UGenericGraphNode*, float>> ValidAttacks;
	double TotalScore = 0.f;
	for (UGenericGraphNode* ChildNode : SourceNode->ChildrenNodes)
	{
		const UAttackNode* AttackNode = CastChecked<UAttackNode>(ChildNode);
		if (AttackNode->GetIsOnCd()) continue;
		const float Priority = AttackNode->GetAttackProperties().Priority;
		ValidAttacks.Add({ChildNode, Priority});
		TotalScore += Priority;
	}

	if(ValidAttacks.IsEmpty()) return nullptr;

	//Randomly chose a valid attack
	UAttackNode* ChosenNode = nullptr;
	std::uniform_real_distribution Distribution(0.0, TotalScore);
	double RandomNumber = Distribution(RandomGenerator);
	for (const TTuple<UGenericGraphNode*, float>& Attack : ValidAttacks)
	{
		RandomNumber -= Attack.Value;
		if (RandomNumber <= 0.f)
		{
			ChosenNode = CastChecked<UAttackNode>(Attack.Key);
			break;
		}
	}
	return ChosenNode;
}

UAttackNode* AOpponentCharacter::GetRandomValidAttackInRange() const
{
	if(!GetAcceptedInputs().IsAllowedInput(EInputType::Attack)) return nullptr;

	const ACharacter* TargetCharacter = GetCombatTarget();
	if (TargetCharacter == nullptr)
	{
		return nullptr;
	}

	const float RequiredRange = FVector::Distance(TargetCharacter->GetActorLocation(), GetActorLocation());
	const UGenericGraphNode* SourceNode = GetCharacterStats()->Attacks.GetCurrentNode(GetWorld());
	
	//Sort through all available attacks and remove those that cannot be executed
	TArray<TTuple<UGenericGraphNode*, float>> ValidAttacks;
	double TotalScore = 0.0;
	for (UGenericGraphNode* ChildNode : SourceNode->ChildrenNodes)
	{
		const UAttackNode* AttackNode = CastChecked<UAttackNode>(ChildNode);
		const FAttackProperties& AttackProperties = AttackNode->GetAttackProperties();
		if (AttackNode->GetIsOnCd() ||
			AttackProperties.MaximalMovementDistance < RequiredRange) continue;
		const float Priority = AttackProperties.Priority;
		ValidAttacks.Add({ChildNode, Priority});
		TotalScore += Priority;
	}

	if(ValidAttacks.IsEmpty()) return nullptr;

	//Randomly chose a valid attack
	UAttackNode* ChosenNode = nullptr;
	std::uniform_real_distribution Distribution(0.0, TotalScore);
	double RandomNumber = Distribution(RandomGenerator);
	//float RandomNumber = FMath::FRandRange(0.0, TotalScore);
	for (const TTuple<UGenericGraphNode*, float>& Attack : ValidAttacks)
	{
		RandomNumber -= Attack.Value;
		if (RandomNumber <= 0.f)
		{
			ChosenNode = CastChecked<UAttackNode>(Attack.Key);
			break;
		}
	}
	return ChosenNode;
}

float AOpponentCharacter::GenerateAggressionScore(APlayerCharacter* PlayerCharacter) const
{
	if(!bCanBecomeAggressive) return -1.f;
	float Score = 0.f;
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
	CharacterStats->Attacks.OnModeChanged.BindUObject(this, &AOpponentCharacter::OnAttackTreeRootChanged);
	if(IsValid(ToughnessBrokenAnimation)) ToughnessBrokenTime = ToughnessBrokenAnimation->GetPlayLength();

	HealthWidgetComponent->OnHealthMonitorWidgetInitialized.AddDynamic(this, &AOpponentCharacter::RegisterHealthInfoWidget);
	Super::BeginPlay();
}

bool AOpponentCharacter::TriggerDeath()
{
	if(!Super::TriggerDeath()) return false;
	bCanBecomeAggressive = false;
	return true;
}

void AOpponentCharacter::GetStaggered(bool HeavyStagger)
{
	Super::GetStaggered(false);
	CharacterStats->ReduceToughness(5);
}

bool AOpponentCharacter::TriggerToughnessBroken()
{
	if(!Super::TriggerToughnessBroken()) return false;
	PlayAnimMontage(ToughnessBrokenAnimation);
	CharacterStats->ChangeHealthByPercentage(-10.f);
	return true;
}

void AOpponentCharacter::RestoreToughness()
{
	Super::RestoreToughness();
}

void AOpponentCharacter::OnGetAttacked(const FAttackDamageEvent* DamageEvent)
{
	PlayHitSound(DamageEvent->HitLocation);
	SpawnHitFX(DamageEvent->HitLocation, DamageEvent->HitFXScaleFactor);

	//opponents can get staggered by attacks (without toughness break)
	std::uniform_int_distribution<int32> Distribution(0.0, 1000.0);
	int32 CasePerThousand = Distribution(RandomGenerator);
	const bool AttackStaggers = CasePerThousand <= DamageEvent->StaggerChance;
	if(AttackStaggers)
	{
		CasePerThousand = Distribution(RandomGenerator); //generate another random number to see if the stagger can be prevented
		const bool IsStaggerPrevented = CasePerThousand <= CharacterStats->InterruptionResistance.GetResulting();
		if(!IsStaggerPrevented) GetStaggered(false);
	}
		
	// ReSharper disable once CppExpressionWithoutSideEffects
	DamageEvent->OnHitRegistered.ExecuteIfBound(AttackStaggers);
	OnHitTimeDilation(AttackStaggers);
}

float AOpponentCharacter::CanAttackInSeconds() const
{
	const float ShortestCd = EarliestAttackSeconds(CharacterStats->Attacks.GetCurrentNode(GetWorld()));
	const float RemainingComboTime = CharacterStats->Attacks.GetComboExpirationTime() - GetWorld()->RealTimeSeconds;

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
		const float RemainingCdTime = CastChecked<UAttackNode>(ChildNode)->CdTimeRemaining();
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
	if(!IsValid(RequiredSpaceActiveCombat) || !IsValid(RequiredSpacePassive)) return;
	RequiredSpaceActiveCombat->ComponentTags.AddUnique(RequiredSpaceActiveTag);
	RequiredSpacePassive->ComponentTags.Remove(RequiredSpaceActiveTag);
}

void AOpponentCharacter::SetUsePassiveSpace() const
{
	if(!IsValid(RequiredSpaceActiveCombat) || !IsValid(RequiredSpacePassive)) return;
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

