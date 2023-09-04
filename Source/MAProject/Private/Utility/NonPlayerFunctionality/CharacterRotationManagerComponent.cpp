// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterRotationManagerComponent.h"

#include <Characters/AdvancedCharacterMovementComponent.h>

#include "NavigationPath.h"
#include "Characters/Fighters/Opponents/OpponentCharacter.h"
#include "Characters/Fighters/Opponents/AI/OpponentController.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values for this component's properties
UCharacterRotationManagerComponent::UCharacterRotationManagerComponent() :
	CharacterRotationMode(ECharacterRotationMode::OrientToMovement),
	OldCharacterRotationMode(ECharacterRotationMode::FlickBack),
	OpponentCharacter(nullptr), OpponentController(nullptr), OldTarget(nullptr)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterRotationManagerComponent::SwitchToOptimal(const FVector& TargetLocation, FNavPathSharedPtr* NavPath)
{
	AActor* LookAtGoal = OpponentCharacter->GetTargetPlayer();
	if(IsValid(LookAtGoal))
	{
		const FVector CurrentToTarget = TargetLocation - GetComponentLocation();
		float DistanceFromTarget;
		FVector DirectionToTarget;
		CurrentToTarget.ToDirectionAndLength(DirectionToTarget, DistanceFromTarget);
		
		const FVector CurrentToLookGoal = LookAtGoal->GetActorLocation() - GetComponentLocation();
		float LongestDistanceToLookGoal;
		FVector DirectionToLookGoal;
		CurrentToLookGoal.ToDirectionAndLength(DirectionToLookGoal, LongestDistanceToLookGoal);

		//determine the truly furthest away point
		for(const FNavPathPoint& PathPoint : NavPath->Get()->GetPathPoints())
		{
			const float DistanceToLookGoal = FVector::Distance(PathPoint.Location, LookAtGoal->GetActorLocation());
			if(DistanceToLookGoal > LongestDistanceToLookGoal) LongestDistanceToLookGoal = DistanceToLookGoal;
		}
		
		float PathLength = NavPath->Get()->GetLength();
		//sanity check
		if(PathLength < DistanceFromTarget) PathLength = DistanceFromTarget;
		

		constexpr float MaxEffectRange = 5000.f;
		constexpr float MaxMovementDistance = 2000.f;
		constexpr float MaxBackwardsDistance = 500.f;
		if(FVector::Distance(TargetLocation, LookAtGoal->GetActorLocation()) < MaxEffectRange &&
			PathLength < MaxMovementDistance && LongestDistanceToLookGoal < MaxEffectRange)
		{
			constexpr float RelevantAngle = 45.f;
			
			const float AngleToTarget = UKismetMathLibrary::DegAcos(
				FVector::DotProduct(DirectionToLookGoal, DirectionToTarget));
			
			//Walking in the direction of the player
			if(AngleToTarget <= RelevantAngle && AngleToTarget >= 0.f)
			{
				SetRotationMode(ECharacterRotationMode::OrientToTarget, true, LookAtGoal);
				return;
			}
			//Walking away from the player
			if(AngleToTarget <= 180.f && AngleToTarget >= 180.f - RelevantAngle)
			{
				//only when walking short distances it makes sense to walk backwards
				if(DistanceFromTarget <= MaxBackwardsDistance)
				{
					SetRotationMode(ECharacterRotationMode::OrientToTarget, true, LookAtGoal);
					return;
				}
			}
		}		
	}
	
	SetRotationMode(ECharacterRotationMode::OrientToMovement, true);
}

void UCharacterRotationManagerComponent::SetRotationMode(ECharacterRotationMode NewRotationMode, bool StoreForFlickBack,
                                                         AActor* NewTarget)
{
	if(!IsValid(OpponentController))
	{
		OpponentController = Cast<AAIController>(OpponentCharacter->GetController());
		if(!IsValid(OpponentController)) return;
	}


	//Store the relevant values so we can reset to the current state later
	if(StoreForFlickBack)
	{
		OldCharacterRotationMode = CharacterRotationMode;
		if(CharacterRotationMode == ECharacterRotationMode::OrientToTarget)
		{
			OldTarget = OpponentController->GetFocusActor();
		}		
	}
	else if(NewRotationMode != ECharacterRotationMode::FlickBack)
	{
		OldCharacterRotationMode = ECharacterRotationMode::FlickBack; //== invalid (in this context)
		OldTarget = nullptr;
	}

	//TODO: transition is sometimes too fast
	switch(NewRotationMode)
	{
	case ECharacterRotationMode::OrientToMovement:
		{
			OpponentCharacter->GetCharacterMovement()->bUseControllerDesiredRotation = false;
			OpponentCharacter->GetCharacterMovement()->bOrientRotationToMovement = true;
			OpponentCharacter->bUseControllerRotationYaw = false;
			break;
		}
	case ECharacterRotationMode::OrientToTarget:
		{
			check(IsValid(NewTarget));
			OpponentCharacter->GetCharacterMovement()->bUseControllerDesiredRotation = true;
			OpponentCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
			OpponentCharacter->bUseControllerRotationYaw = true;
			OpponentController->SetFocus(NewTarget);
			break;
		}
	case ECharacterRotationMode::FlickBack:
		{
			if(OldCharacterRotationMode == ECharacterRotationMode::FlickBack) return;
			SetRotationMode(OldCharacterRotationMode, false, OldTarget);
		}
		break;
	default: checkNoEntry();
	}
}


void UCharacterRotationManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	OpponentCharacter = CastChecked<AOpponentCharacter>(GetOwner());
}

