// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/GeneralCharacter.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Utility/Animation/SuckToTargetComponent.h"
#include "Utility/Stats/StatusEffect.h"


AGeneralCharacter::AGeneralCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer),
	bAllowAutomaticOpacityChanges(true), MinimumFadeDistance(100.f), MaximumFadeDistance(150.f), InputFadeStrength(2.f)
{
	SuckToTargetComponent = CreateDefaultSubobject<USuckToTargetComponent>(TEXT("SuckToTargetComp"));
	PrimaryActorTick.bCanEverTick = true;
	
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionProfileName("CharacterMesh", true);
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

void AGeneralCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	FadeMeshWithCameraDistance();
}

#if WITH_EDITORONLY_DATA
void AGeneralCharacter::SetIsDebugging(bool IsDebugging)
{
	bIsDebugging = IsDebugging;
	SuckToTargetComponent->bIsDebugging = bIsDebugging;
}
#endif

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

void AGeneralCharacter::ReceiveStatusEffect(const TSubclassOf<UStatusEffect>& NewEffectType)
{
	UStatusEffect* NewEffect = DuplicateObject<UStatusEffect>(NewEffectType->GetDefaultObject<UStatusEffect>(), this);
	NewEffect->RegisterComponent();
	NewEffect->OnEffectApplied(this);
}

void AGeneralCharacter::RemoveStatusEffect(const TSubclassOf<UStatusEffect>& EffectType)
{
	TArray<UActorComponent*> MatchingStatusEffects;
	GetComponents(EffectType, MatchingStatusEffects);
	if(MatchingStatusEffects.IsEmpty()) return;
	MatchingStatusEffects[0]->DestroyComponent();
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

void AGeneralCharacter::BeginPlay()
{
	Super::BeginPlay();
	CameraPlayerController = GetWorld()->GetFirstPlayerController();
}

