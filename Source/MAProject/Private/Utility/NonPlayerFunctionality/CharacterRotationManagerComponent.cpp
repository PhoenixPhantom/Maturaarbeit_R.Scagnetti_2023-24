// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterRotationManagerComponent.h"

#include <Characters/AdvancedCharacterMovementComponent.h>

#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "Characters/Fighters/Opponents/OpponentCharacter.h"
#include "Characters/Fighters/Opponents/AI/OpponentController.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values for this component's properties
UCharacterRotationManagerComponent::UCharacterRotationManagerComponent() : bIsInCombat(false),
	CharacterRotationMode(ECharacterRotationMode::OrientToMovement),
	StoredCharacterRotationMode(ECharacterRotationMode::FlickBack), OpponentCharacter(nullptr),
	OpponentController(nullptr), StoredTarget(nullptr)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UCharacterRotationManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(bIsInCombat)
	{
		//end combat for the rotation manager if the owing character has no combat target anymore
		if(!IsValid(OpponentCharacter->GetCombatTarget()))
		{
			PrimaryComponentTick.SetTickFunctionEnable(false);
			return;
		}
		if(FVector::Distance(GetComponentLocation(), OpponentCharacter->GetCombatTarget()->GetActorLocation()) <=
					OpponentCharacter->GetPassivePlayerDistanceConstraint().MaxRadius)
		{
			SetRotationMode(ECharacterRotationMode::OrientToTarget, false,
				OpponentCharacter->GetCombatTarget());
		}
		else
		{
			SetRotationMode(ECharacterRotationMode::FlickBack);
		}
	}
	else
	{
		const FVector& Direction = UKismetMathLibrary::GetDirectionUnitVector(OpponentCharacter->GetActorLocation(),
			OpponentController->GetFocalPoint());
		if(UKismetMathLibrary::DegAcos(FVector::DotProduct(Direction, OpponentCharacter->GetActorForwardVector())) <= 10.f)
		{
			PrimaryComponentTick.SetTickFunctionEnable(false);
			SetRotationMode(ECharacterRotationMode::FlickBack, false);
		}
	}
}

void UCharacterRotationManagerComponent::ChooseOptimalForCombat(const FVector& TargetLocation)
{
	bIsInCombat = true;
	AActor* LookAtGoal = OpponentCharacter->GetCombatTarget();
	if(IsValid(LookAtGoal))
	{
		const float MaxCombatRadius = OpponentCharacter->GetPassivePlayerDistanceConstraint().MaxRadius;
		const FVector& LookAtGoalLocation = LookAtGoal->GetActorLocation();

		const bool IsEndInRange = FVector::Distance(TargetLocation, LookAtGoalLocation) <= MaxCombatRadius;
		const bool IsStartInRange = FVector::Distance(GetComponentLocation(), LookAtGoalLocation) <= MaxCombatRadius;
		bool EverLeavesRange = !IsEndInRange || !IsStartInRange;
		if(IsEndInRange && IsStartInRange)
		{
			//Also: all path points have to be close enough, to guarantee,
			//that we don't make a long detour to get around some obstacle
			UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetCurrent(GetWorld());
			UNavigationPath* NavigationPath = NavigationSystem->FindPathToLocationSynchronously(GetWorld(),
				GetComponentLocation(), TargetLocation);
			if(IsValid(NavigationPath) && NavigationPath->GetPath().IsValid()){
				for(const FNavPathPoint& PathPoint : NavigationPath->GetPath()->GetPathPoints())
				{
					if(FVector::Distance(PathPoint.Location, LookAtGoalLocation) > MaxCombatRadius)
					{
						EverLeavesRange = true;
						break;
					}
				}
			}
		}
		if(!EverLeavesRange)
		{
			SetRotationMode(ECharacterRotationMode::OrientToTarget, false, LookAtGoal);
			PrimaryComponentTick.SetTickFunctionEnable(false);
			return;
		}
		if(IsEndInRange)
		{
			SetRotationMode(ECharacterRotationMode::OrientToMovement, false);
			PrimaryComponentTick.SetTickFunctionEnable(true);
			return;
		}
		//if we start in range but end outside of it, directly turning around makes sense
	}
	
	SetRotationMode(ECharacterRotationMode::OrientToMovement, false);
	PrimaryComponentTick.SetTickFunctionEnable(false);
}

void UCharacterRotationManagerComponent::SetRotationMode(ECharacterRotationMode NewRotationMode, bool StoreForFlickBack,
                                                         AActor* NewTarget, const FVector& TargetLocation)
{
	if(!IsValid(OpponentController))
	{
		OpponentController = Cast<AAIController>(OpponentCharacter->GetController());
		if(!IsValid(OpponentController)) return;
	}

	//TODO: transition is sometimes too fast
	switch(NewRotationMode)
	{
	case ECharacterRotationMode::OrientToMovement:
		{
			OpponentCharacter->GetCharacterMovement()->bUseControllerDesiredRotation = true;
			OpponentCharacter->GetCharacterMovement()->bOrientRotationToMovement = true;
			OpponentController->bSetControlRotationFromPawnOrientation = true;
			OpponentController->ClearFocus(EAIFocusPriority::Gameplay);
			break;
		}
	case ECharacterRotationMode::OrientToTarget:
		{
			OpponentCharacter->GetCharacterMovement()->bUseControllerDesiredRotation = true;
			OpponentCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
			OpponentController->bSetControlRotationFromPawnOrientation = false;
			if(IsValid(NewTarget)) OpponentController->SetFocus(NewTarget);
			else
			{
 				OpponentController->SetFocalPoint(TargetLocation);
			}
			break;
		}
	case ECharacterRotationMode::FlickBack:
		{
			if(StoredCharacterRotationMode == ECharacterRotationMode::FlickBack || !IsValid(StoredTarget)) return;
			SetRotationMode(StoredCharacterRotationMode, false, StoredTarget);
		}
		break;
	default: checkNoEntry();
	}
	CharacterRotationMode = NewRotationMode;

	//Store the relevant values so we can reset to the current state later
	if(StoreForFlickBack)
	{
		StoredCharacterRotationMode = CharacterRotationMode;
		if(CharacterRotationMode == ECharacterRotationMode::OrientToTarget)
		{
			StoredTarget = OpponentController->GetFocusActor();
		}		
	}
}


void UCharacterRotationManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	OpponentCharacter = CastChecked<AOpponentCharacter>(GetOwner());
}

