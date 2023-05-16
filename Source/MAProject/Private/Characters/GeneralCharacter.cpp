// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/GeneralCharacter.h"
#include "MotionWarpingComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Utility/Animation/SuckToTargetComponent.h"


// Sets default values
AGeneralCharacter::AGeneralCharacter()
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
bool AGeneralCharacter::AreMultipleVisible(AActor* Target, const FVector& TraceStart, TArray<FVector>& RemainingEnds,
                                           int32 RequiredPositiveTests)
{
	if(RequiredPositiveTests <= 0) return true;
	if(RemainingEnds.IsEmpty()) return false;
	FHitResult VisibilityTrace;
	UKismetSystemLibrary::LineTraceSingle(GetWorld(), TraceStart, RemainingEnds[0],
	ETraceTypeQuery::TraceTypeQuery1, true, {this, Owner}, EDrawDebugTrace::None,
	VisibilityTrace, true);
	if(!VisibilityTrace.bBlockingHit || VisibilityTrace.GetActor() == Target) RequiredPositiveTests--;
	RemainingEnds.RemoveAt(0);
	return AreMultipleVisible(Target, TraceStart, RemainingEnds, RequiredPositiveTests);
}

