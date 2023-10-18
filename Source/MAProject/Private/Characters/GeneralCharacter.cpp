// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/GeneralCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Utility/Animation/SuckToTargetComponent.h"




AGeneralCharacter::AGeneralCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SuckToTargetComponent = CreateDefaultSubobject<USuckToTargetComponent>(TEXT("SuckToTargetComp"));
	PrimaryActorTick.bCanEverTick = false;
	
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionProfileName("CharacterMesh", true);
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

#if WITH_EDITORONLY_DATA
void AGeneralCharacter::SetIsDebugging(bool IsDebugging)
{
	bIsDebugging = IsDebugging;
	SuckToTargetComponent->bIsDebugging = bIsDebugging;
}
#endif

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

