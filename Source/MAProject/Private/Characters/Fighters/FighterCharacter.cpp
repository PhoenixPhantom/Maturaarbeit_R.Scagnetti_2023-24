// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/FighterCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

AFighterCharacter::AFighterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AFighterCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if(!MeleeEnabledBones.IsEmpty()) CheckMeshOverlaps();
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

void AFighterCharacter::ActivateMeleeBones(const TArray<FName>& BonesToEnable, bool StartEmpty,
	bool AllowHitRecentVicitms, FMeleeControlsKey Key)
{
	if(StartEmpty) MeleeEnabledBones.Empty();
	if(AllowHitRecentVicitms) RecentlyDamagedActors.Empty();
	MeleeEnabledBones.Append(BonesToEnable);
}

void AFighterCharacter::DeactivateMeleeBones(const TArray<FName>& BonesToDisable, bool RefreshHitActors,
	FMeleeControlsKey Key)
{
	if(RefreshHitActors) RecentlyDamagedActors.Empty();
	for(FName BoneToDisable : BonesToDisable) MeleeEnabledBones.Remove(BoneToDisable);
}

void AFighterCharacter::SwitchMovementToWalk() const
{
	GetCharacterMovement()->MaxWalkSpeed = CharacterStats->WalkSpeed.GetResulting();
}

void AFighterCharacter::SwitchMovementToRun() const
{
	GetCharacterMovement()->MaxWalkSpeed = CharacterStats->RunSpeed.GetResulting();
}

void AFighterCharacter::CheckMeshOverlaps()
{
	TArray<AActor*> OverlappingActors;
	GetMesh()->GetOverlappingActors(OverlappingActors);
	
	//loop through all bones that can damage targets, to find some that actually hit a target
	FHitResult TraceResult;
	for(FName BodyName : MeleeEnabledBones)
	{
		FVector Velocity = GetMesh()->GetBoneLinearVelocity(BodyName);
		Velocity.Normalize();

		//we need hit results for attack management
		//tracing on TraceTypeQuery6 (== Destructible)
		UKismetSystemLibrary::LineTraceSingle(GetWorld(), GetMesh()->GetBoneLocation(BodyName),
		GetMesh()->GetBoneLocation(BodyName) + Velocity * 100.f, ETraceTypeQuery::TraceTypeQuery6,
		true, {this, Owner}, EDrawDebugTrace::None, TraceResult, true);

		if(TraceResult.bBlockingHit)
		{
			for(AActor* Target : OverlappingActors)
			{
				if(Target == this || !IsValid(Target)  || RecentlyDamagedActors.Contains(Target) ||
					TraceResult.GetActor() != Target) continue;
				RecentlyDamagedActors.Add(Target);
				Target->TakeDamage(CharacterStats->GetDamageOutput(),
					*CharacterStats->GenerateDamageEvent(TraceResult), GetController(), this);
				break;
			}
		}
	}
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
	CharacterStats->OnCheckCanExecuteAttack.BindDynamic(this, &AFighterCharacter::OnCheckCanExecuteAttack);
	CharacterStats->OnGetHit.AddDynamic(this, &AFighterCharacter::OnGetHit);
	CharacterStats->OnNoHealthReached.AddDynamic(this, &AFighterCharacter::OnDeath);
	SwitchMovementToWalk();
}

bool AFighterCharacter::OnCheckCanExecuteAttack(const FAttackProperties& Properties)
{
	return AcceptedInputs.CanOverrideCurrentInput(Properties.InitialLimits.LimiterType) && !GetCharacterMovement()->IsFalling();
}

void AFighterCharacter::OnExecuteAttack(const FAttackProperties& Properties)
{
	StopAnimMontage();
	PlayAnimMontage(Properties.AtkAnimation);
	AcceptedInputs.LimitAvailableInputs(Properties.InitialLimits, Properties.ReducedLimits, GetWorld());
}

void AFighterCharacter::OnGetHit(const FCustomDamageEvent& DamageEvent)
{
	if(!IsValid(GetHitAnimation) || !AcceptedInputs.CanOverrideCurrentInput(EInputType::Stagger)) return;
	StopAnimMontage();
	PlayAnimMontage(GetHitAnimation);
	AcceptedInputs.LimitAvailableInputs(EInputType::Stagger, GetWorld());
}

void AFighterCharacter::OnDeath(const FCustomDamageEvent& DamageEvent)
{
	if(!IsValid(DeathAnimation) || !AcceptedInputs.CanOverrideCurrentInput(EInputType::Death)) return;
	StopAnimMontage();
	PlayAnimMontage(GetHitAnimation);
	AcceptedInputs.LimitAvailableInputs(EInputType::Death, GetWorld());
}
