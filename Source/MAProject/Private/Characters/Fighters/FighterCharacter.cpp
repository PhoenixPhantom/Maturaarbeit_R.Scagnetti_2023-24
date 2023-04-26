// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/FighterCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MAProject/MAProject.h"

AFighterCharacter::AFighterCharacter()
{
	GetMesh()->SetGenerateOverlapEvents(true);
	GetMesh()->OnComponentBeginOverlap.AddDynamic(this, &AFighterCharacter::OnMeshOverlapEvent);
}

void AFighterCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

float AFighterCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	uint32 RemainingHealth = CharacterStats->Health;
	if(DamageEvent.IsOfType(FCustomDamageEvent::ClassID))
	{
		FDamageEvent* Event = const_cast<FDamageEvent*>(&DamageEvent);
		if(DamageEvent.IsOfType(FAttackDamageEvent::ClassID))
			RemainingHealth = CharacterStats->ReceiveDamage(DamageAmount,
				*static_cast<FAttackDamageEvent*>(Event));
		else RemainingHealth = CharacterStats->FGeneralObjectStats::ReceiveDamage(DamageAmount,
			*static_cast<FCustomDamageEvent*>(Event));
	}
	return RemainingHealth;
}

void AFighterCharacter::ActivateMeleeBones(const TArray<FName>& BonesToEnable, FMeleeControlsKey Key)
{
	if(!MeleeEnabledBones.IsEmpty())
	{
		UE_LOG(LogDamageSystem, Warning, TEXT("MeleeEnabledBones was still full when starting a new attack."));
		MeleeEnabledBones.Empty();
	}
	MeleeEnabledBones.Append(BonesToEnable);
}

void AFighterCharacter::DeactivateMeleeBones(const TArray<FName>& BonesToDisable, FMeleeControlsKey Key)
{
	for(FName BoneToDisable : BonesToDisable) MeleeEnabledBones.Remove(BoneToDisable);
	if(!MeleeEnabledBones.IsEmpty())
		UE_LOG(LogDamageSystem, Warning, TEXT("MeleeEnabledBones wasn't emptied completely after the attack was carried out."));

}

void AFighterCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AFighterCharacter::OnMeshOverlapEvent(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(OtherActor == this || !IsValid(OtherActor) || !OtherComp->ComponentTags.Contains(HitReactingVolumeTag))
		return;

	//Overlap events don't generate full hit results but we need them for attack management
	FHitResult TraceResult;
	for(FName BodyName : MeleeEnabledBones)
	{
		FVector Velocity = GetMesh()->GetBoneLinearVelocity(BodyName);
		Velocity.Normalize();

		//not sure if tracing on world dynamic makes sense (TraceTypeQuery2)
		UKismetSystemLibrary::LineTraceSingle(GetWorld(), GetMesh()->GetBoneLocation(BodyName),
		GetMesh()->GetBoneLocation(BodyName) + Velocity * 100.f, ETraceTypeQuery::TraceTypeQuery2,
		true, {this, Owner}, EDrawDebugTrace::None, TraceResult, true);
		if(TraceResult.bBlockingHit && TraceResult.GetActor() == OtherActor)
		{
			RecentlyDamagedActors.Add(OtherActor);
			OtherActor->TakeDamage(CharacterStats->GetDamageOutput(),
				*CharacterStats->GenerateDamageEvent(TraceResult), GetController(), this);
			break;
		}
	}
}
