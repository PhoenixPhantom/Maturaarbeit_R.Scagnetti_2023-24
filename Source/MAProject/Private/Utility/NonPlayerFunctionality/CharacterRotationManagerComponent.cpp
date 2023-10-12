// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterRotationManagerComponent.h"

#include <Characters/AdvancedCharacterMovementComponent.h>

#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "Characters/Fighters/Opponents/OpponentCharacter.h"
#include "Characters/Fighters/Opponents/AI/OpponentController.h"
#include "Kismet/KismetSystemLibrary.h"


// Sets default values for this component's properties
UCharacterRotationManagerComponent::UCharacterRotationManagerComponent() :
	CharacterRotationMode(ECharacterRotationMode::OrientToMovement),
	StoredCharacterRotationMode(ECharacterRotationMode::FlickBack),
	OpponentCharacter(nullptr), OpponentController(nullptr), StoredTarget(nullptr)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterRotationManagerComponent::SwitchToOptimal(const FVector& TargetLocation)
{
	AActor* LookAtGoal = OpponentCharacter->GetTargetPlayer();
	if(IsValid(LookAtGoal))
	{
		const float MaxCombatRadius = OpponentCharacter->GetPassivePlayerDistanceConstraint().MaxRadius;
		const FVector& LookAtGoalLocation = LookAtGoal->GetActorLocation();

		//only consider to keep looking at the target if both the current position as well as the target
		//position are close enough for it to be relevant
		if(FVector::Distance(GetComponentLocation(), LookAtGoalLocation) <= MaxCombatRadius &&
			FVector::Distance(TargetLocation, LookAtGoalLocation) <= MaxCombatRadius)
		{
			bool ShouldLookAtTarget = true;
			//Also: all path points have to be close enough, to guarantee,
			//that we don't make a long detour to get around some obstacle
			UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetCurrent(GetWorld());
			UNavigationPath* NavigationPath = NavigationSystem->FindPathToLocationSynchronously(GetWorld(),
				GetComponentLocation(), TargetLocation);
			for(const FNavPathPoint& PathPoint : NavigationPath->GetPath()->GetPathPoints())
			{
				UKismetSystemLibrary::DrawDebugPoint(GetWorld(), PathPoint.Location, 20.f, FLinearColor(1.f, 0.f, 0.f), 5.f);
				//For some random reason, it sometimes happens that invalid locations are part of the array
				if(FVector::Distance(PathPoint.Location, LookAtGoalLocation) > MaxCombatRadius) // &&
					//PathPoint.Location != FAISystem::InvalidLocation && PathPoint.Location != FVector(0.f))
				{
					GLog->Log(PathPoint.Location.ToString() + " - " + LookAtGoalLocation.ToString() + " > " +
						FString::SanitizeFloat(MaxCombatRadius));
					ShouldLookAtTarget = false;
					break;
				}
			}
			
			if(ShouldLookAtTarget)
			{
				SetRotationMode(ECharacterRotationMode::OrientToTarget, false, LookAtGoal);
			}
			else 
				GLog->Log("Second failed");
		}
		else
		{
			
			GLog->Log(TargetLocation.ToString() + " - " + LookAtGoalLocation.ToString() + " > " +
				FString::SanitizeFloat(MaxCombatRadius));
		}
	}
	
	SetRotationMode(ECharacterRotationMode::OrientToMovement, false);
}

void UCharacterRotationManagerComponent::SetRotationMode(ECharacterRotationMode NewRotationMode, bool StoreForFlickBack,
                                                         AActor* NewTarget)
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
			check(IsValid(NewTarget));
			OpponentCharacter->GetCharacterMovement()->bUseControllerDesiredRotation = true;
			OpponentCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
			OpponentController->bSetControlRotationFromPawnOrientation = false;
			OpponentController->SetFocus(NewTarget);
			break;
		}
	case ECharacterRotationMode::FlickBack:
		{
			if(StoredCharacterRotationMode == ECharacterRotationMode::FlickBack) return;
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

