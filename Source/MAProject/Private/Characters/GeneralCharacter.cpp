// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/GeneralCharacter.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Utility/Animation/CustomAnimInstance.h"
#include "Utility/Animation/SuckToTargetComponent.h"
#include "Utility/Stats/StatusEffect.h"


AGeneralCharacter::AGeneralCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer),
	bAllowAutomaticOpacityChanges(true), MinimumFadeDistance(100.f), MaximumFadeDistance(150.f), InputFadeStrength(2.f)
{
	SuckToTargetComponent = CreateDefaultSubobject<USuckToTargetComponent>(TEXT("SuckToTargetComp"));
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->bDynamicObstacle = false;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionProfileName("CharacterMesh", true);
}

void AGeneralCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(IsValid(CustomAnimInstance))
	{
		const FVector& Velocity = GetCharacterMovement()->Velocity;
		CustomAnimInstance->SetMovement(Velocity);
		CustomAnimInstance->SetLegIKBlend(GetLegIKBlendWeight(Velocity));
	}
	
	
	FadeMeshWithCameraDistance();
}

void AGeneralCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
	if(GetCharacterMovement()->MovementMode == MOVE_Falling || GetCharacterMovement()->MovementMode == MOVE_Flying)
		CharacterInAir();
	if ((PrevMovementMode == MOVE_Falling || PrevMovementMode == MOVE_Flying) &&
		GetCharacterMovement()->MovementMode == MOVE_Walking ||
		GetCharacterMovement()->MovementMode == MOVE_NavWalking ||
		GetCharacterMovement()->MovementMode == MOVE_Swimming)
			CharacterLanded();
}

void AGeneralCharacter::GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	if(!IsValid(GetMesh()))
	{
		Super::GetActorEyesViewPoint(OutLocation, OutRotation);
		return;
	}
	const FName HeadSocket = *(BonePrefix + "-HeadSocket");
	OutLocation = GetMesh()->GetSocketLocation(HeadSocket);
	OutRotation = GetMesh()->GetSocketRotation(HeadSocket);
}

float AGeneralCharacter::GetMeshesOpacity() const
{
	return GetMesh()->GetCustomPrimitiveData().Data.IsEmpty() ? 0.f : GetMesh()->GetCustomPrimitiveData().Data[0];
}

void AGeneralCharacter::SetMeshesOpacity(float DesiredOpacity, FSetCharacterOpacity)
{
	GetMesh()->SetCustomPrimitiveDataFloat(0, DesiredOpacity);
	for(USkeletalMeshComponent* MeshComponent : RelevantMeshes)
	{
		MeshComponent->SetCustomPrimitiveDataFloat(0, DesiredOpacity);
	}
}



#if WITH_EDITORONLY_DATA
void AGeneralCharacter::SetIsDebugging(bool IsDebugging)
{
	bIsDebugging = IsDebugging;
	SuckToTargetComponent->bIsDebugging = bIsDebugging;
}
#endif




void AGeneralCharacter::BeginPlay()
{
	Super::BeginPlay();
	CameraPlayerController = GetWorld()->GetFirstPlayerController();
	CustomAnimInstance = CastChecked<UCustomAnimInstance>(GetMesh()->GetAnimInstance());
}

float AGeneralCharacter::GetLegIKBlendWeight(const FVector& Velocity)
{
	return 1.f - FMath::Min(Velocity.Length() / 400.f, 1.f);
}

void AGeneralCharacter::FadeMeshWithCameraDistance()
{
	if(!bAllowAutomaticOpacityChanges) return;
	if(!IsValid(CameraPlayerController))
	{
		CameraPlayerController = GetWorld()->GetFirstPlayerController();
		if(!IsValid(CameraPlayerController)) return;
	}
	FVector CameraLocation;
	FRotator CameraRotation;
	CameraPlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
	const float OldOpacity = GetMeshesOpacity();
	const float Distance = FVector::Distance(CameraLocation, GetActorLocation());
	if(Distance > MaximumFadeDistance)
	{
		if(OldOpacity != 0)
		{
			SetMeshesOpacity(0.f, FSetCharacterOpacity());
		}
	}
	else if(Distance < MinimumFadeDistance)
	{
		if(OldOpacity != 1.f) SetMeshesOpacity(1.f, FSetCharacterOpacity());	
	}
	else
	{
		const float DesiredOpacity = 1.f -
			pow((Distance - MinimumFadeDistance) / (MaximumFadeDistance - MinimumFadeDistance),InputFadeStrength);
		if(abs(DesiredOpacity - OldOpacity) >  0.001) SetMeshesOpacity(DesiredOpacity, FSetCharacterOpacity());
	}
}

void AGeneralCharacter::CharacterLanded()
{
	if(IsValid(CustomAnimInstance))
	{
		CustomAnimInstance->SetIsInAir(false);
	}
}

void AGeneralCharacter::CharacterInAir()
{
	if(IsValid(CustomAnimInstance))
	{
		CustomAnimInstance->SetIsInAir(true);
	}
}


bool AGeneralCharacter::TriggerDeath()
{
	if(!IsValid(CustomAnimInstance) || !AcceptedInputs.IsAllowedInput(EInputType::Death) ||
		GetMesh()->GetCollisionEnabled() == ECollisionEnabled::NoCollision) return false;
	CustomAnimInstance->TriggerDeath();
	
	AcceptedInputs.LimitAvailableInputs({EInputType::Death, CustomAnimInstance->GetDeathAnimTime()}, GetWorld());
	TDelegate<void(bool)> OnDeathDelegate;
	OnDeathDelegate.BindWeakLambda(this, [this](bool IsLimitDurationOver)
	{
		Destroy();
	});
	AcceptedInputs.OnInputLimitsReset.Add(OnDeathDelegate);
	return true;
}

void AGeneralCharacter::RemoveStatusEffectInternal(UStatusEffect* StatusEffect)
{
	StatusEffect->OnEffectRemoved_Implementation(this);
	StatusEffect->DestroyComponent();
	OnStatusEffectRemoved();
}

bool AGeneralCharacter::AreMultipleVisible(AActor* Target, ETraceTypeQuery TraceType, const FVector& TraceStart,
                                           TArray<FVector>& RemainingEnds, int32 RequiredPositiveTests) const
{
	if(RequiredPositiveTests <= 0) return true;
	if(RemainingEnds.IsEmpty()) return false;
	FHitResult VisibilityTrace;
	UKismetSystemLibrary::LineTraceSingle(GetWorld(), TraceStart, RemainingEnds[0], TraceType, true,
	{const_cast<AGeneralCharacter*>(this), Owner}, EDrawDebugTrace::None,
	VisibilityTrace, true);
	if(!VisibilityTrace.bBlockingHit || VisibilityTrace.GetActor() == Target) RequiredPositiveTests--;
	RemainingEnds.RemoveAt(0);
	return AreMultipleVisible(Target, TraceType, TraceStart, RemainingEnds, RequiredPositiveTests);
}

void AGeneralCharacter::ReceiveStatusEffect(TSubclassOf<UStatusEffect> NewEffectType)
{
	TArray<UActorComponent*> MatchingStatusEffects;
	GetComponents(NewEffectType, MatchingStatusEffects);

	//Re-adding a status effect refreshes only it's duration (but doesn't add the effect twice)
	if(MatchingStatusEffects.IsEmpty())
	{
		UStatusEffect* TargetEffect = NewObject<UStatusEffect>(this, NewEffectType);
		TargetEffect->RegisterComponent();
		TargetEffect->OnEffectApplied_Implementation(this);
		OnNewStatusEffectReceived(TargetEffect);
		return;
	}
	
	CastChecked<UStatusEffect>(MatchingStatusEffects[0])->ForceRestartTimer(FForceStatusEffectTimerRestartKey());
}

void AGeneralCharacter::RemoveStatusEffect(TSubclassOf<UStatusEffect> EffectType)
{
	TArray<UActorComponent*> MatchingStatusEffects;
	GetComponents(EffectType, MatchingStatusEffects);
	if(MatchingStatusEffects.IsEmpty()) return;
	RemoveStatusEffectInternal(CastChecked<UStatusEffect>(MatchingStatusEffects[0]));
}


void AGeneralCharacter::RegisterRelevantMeshes(const TArray<USkeletalMeshComponent*>& NewMeshes, bool AddToBaseMesh,
                                               bool ForceUpdate)
{
	RelevantMeshes.Append(NewMeshes);
	if(AddToBaseMesh)
	{
		for(USkeletalMeshComponent* NewMesh : NewMeshes)
		{
			NewMesh->SetLeaderPoseComponent(GetMesh(), ForceUpdate);
		}
	}
}