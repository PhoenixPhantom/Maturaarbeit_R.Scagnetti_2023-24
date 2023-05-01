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

void AFighterCharacter::ActivateMeleeBones(const TArray<FName>& BonesToEnable, bool StartEmpty, FMeleeControlsKey Key)
{
	if(StartEmpty) MeleeEnabledBones.Empty();
	MeleeEnabledBones.Append(BonesToEnable);
}

void AFighterCharacter::DeactivateMeleeBones(const TArray<FName>& BonesToDisable, bool RefreshHitActors, FMeleeControlsKey Key)
{
	if(RefreshHitActors) RecentlyDamagedActors.Empty();
	for(FName BoneToDisable : BonesToDisable) MeleeEnabledBones.Remove(BoneToDisable);
}

void AFighterCharacter::ExecuteAttack(int32 Index)
{
	CharacterStats->ExecuteAttack(Index);
}

void AFighterCharacter::BeginPlay()
{
	Super::BeginPlay();
	check(GetMesh()->GetRelativeTransform().GetMaximumAxisScale() == GetMesh()->GetRelativeTransform().GetMinimumAxisScale());
	SetAnimRootMotionTranslationScale(GetMesh()->GetRelativeTransform().GetMaximumAxisScale()/100.f);
	CharacterStats->OnExecuteAttack.AddDynamic(this, &AFighterCharacter::OnExecuteAttack);
	CharacterStats->OnGetHit.AddDynamic(this, &AFighterCharacter::OnGetHit);
	CharacterStats->OnNoHealthReached.AddDynamic(this, &AFighterCharacter::OnDeath);
}

void AFighterCharacter::OnExecuteAttack(const FAttackProperties& Properties)
{
	if(!AcceptedInputs.CanOverrideCurrentInput(Properties.ResultingLimits.LimiterType)) return;
	PlayAnimMontage(Properties.AtkAnimation);
	AcceptedInputs.LimitAvailableInputs(Properties.ResultingLimits, GetWorld());
}

void AFighterCharacter::OnGetHit(const FCustomDamageEvent& DamageEvent)
{
	if(!IsValid(GetHitAnimation) || !AcceptedInputs.CanOverrideCurrentInput(EInputType::Stagger)) return;
	StopAnimMontage(nullptr);
	PlayAnimMontage(GetHitAnimation);
	AcceptedInputs.LimitAvailableInputs(EInputType::Stagger, GetWorld());
}

void AFighterCharacter::OnDeath(const FCustomDamageEvent& DamageEvent)
{
	if(!IsValid(DeathAnimation) || !AcceptedInputs.CanOverrideCurrentInput(EInputType::Death)) return;
	StopAnimMontage(nullptr);
	PlayAnimMontage(GetHitAnimation);
	AcceptedInputs.LimitAvailableInputs(EInputType::Death, GetWorld());
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

		//not sure if tracing on TraceTypeQuery3 (== whatever) makes sense (TraceTypeQuery2 == WorldDynamic, TraceTypeQuery1 == Visibility, Camera)
		
		UKismetSystemLibrary::LineTraceSingle(GetWorld(), GetMesh()->GetBoneLocation(BodyName),
		GetMesh()->GetBoneLocation(BodyName) + Velocity * 100.f, ETraceTypeQuery::TraceTypeQuery3,
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
