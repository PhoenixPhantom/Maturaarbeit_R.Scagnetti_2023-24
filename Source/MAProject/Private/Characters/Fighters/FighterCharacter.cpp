// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/FighterCharacter.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/Animation/SuckToTargetComponent.h"
#include "Utility/NonPlayerFunctionality/TargetInformationComponent.h"

AFighterCharacter::AFighterCharacter() : HitFXRadius(50.f)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	GetMesh()->SetGenerateOverlapEvents(true);
	
	TargetInformationComponent = CreateDefaultSubobject<UTargetInformationComponent>(TEXT("TargetInformationComp"));
	TargetInformationComponent->SetupAttachment(RootComponent);
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
		{
			const FAttackDamageEvent AttackDamageEvent = *static_cast<FAttackDamageEvent*>(Event);
			RemainingHealth = CharacterStats->ReceiveDamage(DamageAmount, AttackDamageEvent);

			//Spawn get hit FX
			constexpr float RealRadius = 75.f;
			FFXSystemSpawnParameters SpawnParameters;
			SpawnParameters.SystemTemplate = GetHitFX;
			SpawnParameters.Location = AttackDamageEvent.HitLocation;
			SpawnParameters.Scale = FVector(HitFXRadius/RealRadius * AttackDamageEvent.HitFXScaleFactor);
			SpawnParameters.AttachToComponent = GetRootComponent();
			SpawnParameters.bAutoActivate = true;
			SpawnParameters.bAutoDestroy = true;
			SpawnParameters.WorldContextObject = GetWorld();
			UNiagaraComponent* NiagaraComponent =
				UNiagaraFunctionLibrary::SpawnSystemAttachedWithParams(SpawnParameters);
			NiagaraComponent->SetVariableLinearColor("BaseColor", FLinearColor(0.5f, 0.5f, 0.5f));
		}
			
		else RemainingHealth = CharacterStats->FGeneralObjectStats::ReceiveDamage(DamageAmount,
			*static_cast<FCustomDamageEvent*>(Event));
	}
	return RemainingHealth;
}

bool AFighterCharacter::IsMovingOnFloor() const
{
	return GetCharacterMovement()->MovementMode == MOVE_Walking ||
		GetCharacterMovement()->MovementMode == MOVE_NavWalking;
}

bool AFighterCharacter::IsWalking() const
{
	return IsMovingOnFloor() &&
		GetCharacterMovement()->GetMaxSpeed() == CharacterStats->WalkSpeed.GetResulting();
}

bool AFighterCharacter::IsRunning() const
{
	return IsMovingOnFloor() &&
		GetCharacterMovement()->GetMaxSpeed() == CharacterStats->RunSpeed.GetResulting();

}

void AFighterCharacter::ActivateMeleeBones(const TArray<FName>& BonesToEnable, bool StartEmpty,
                                           bool AllowHitRecentVictims, FMeleeControlsKey Key)
{
	if(StartEmpty) MeleeEnabledBones.Empty();
	if(AllowHitRecentVictims) RecentlyDamagedActors.Empty();
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
	for(const FName BodyName : MeleeEnabledBones)
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
					CharacterStats->GenerateDamageEvent(TraceResult), GetController(), this);
				break;
			}
		}
	}
}

void AFighterCharacter::QueueFollowUpLimit(const TArray<FInputLimits>& InputLimits, int32 CurrentLimitIndex)
{
	int32 Index = CurrentLimitIndex + 1;
	if(!InputLimits.IsValidIndex(Index)) return;
	AcceptedInputs.OnInputLimitsReset.AddLambda([InputLimits, Index, World = GetWorld(), this]
		(bool IsLimitDurationOver, bool& HasBeenCleared)
	{
		AcceptedInputs.LimitAvailableInputs(InputLimits[Index], World);
		if(!HasBeenCleared)
		{
			AcceptedInputs.OnInputLimitsReset.Clear();
			HasBeenCleared = true;
		}
		QueueFollowUpLimit(InputLimits, Index);
	});
}

bool AFighterCharacter::ExecuteAttack(const FAttackProperties& AttackProperties)
{
	const int32 Index = CharacterStats->AvailableAttacks.Find(AttackProperties);
	if(Index == INDEX_NONE) return false;
	ExecuteAttack(Index);
	return true;
}

void AFighterCharacter::ExecuteAttack(int32 Index)
{
	CharacterStats->ExecuteAttack(Index);
}

void AFighterCharacter::BeginPlay()
{
	Super::BeginPlay();
	check(GetMesh()->GetRelativeTransform().GetMaximumAxisScale() == GetMesh()->GetRelativeTransform().GetMinimumAxisScale());
	SuckToTargetComponent->SetupReferences(this);
	SetAnimRootMotionTranslationScale(GetMesh()->GetRelativeTransform().GetMaximumAxisScale()/100.f);
	CharacterStats->OnExecuteAttack.AddDynamic(this, &AFighterCharacter::OnExecuteAttack);
	CharacterStats->OnCheckCanExecuteAttack.BindDynamic(this, &AFighterCharacter::OnCheckCanExecuteAttack);
	CharacterStats->OnGetHit.AddDynamic(this, &AFighterCharacter::OnGetHit);
	CharacterStats->OnNoHealthReached.AddDynamic(this, &AFighterCharacter::OnDeath);
	SwitchMovementToWalk();
}

bool AFighterCharacter::OnCheckCanExecuteAttack(const FAttackProperties& Properties)
{
	return AcceptedInputs.CanOverrideCurrentInput(Properties.InputLimits[0].LimiterType) && !GetCharacterMovement()->IsFalling();
}

void AFighterCharacter::OnExecuteAttack(const FAttackProperties& Properties)
{
	StopAnimMontage();
	PlayAnimMontage(Properties.AtkAnimation);
	AcceptedInputs.LimitAvailableInputs(Properties.InputLimits[0], GetWorld());
	QueueFollowUpLimit(Properties.InputLimits);
}

void AFighterCharacter::OnGetHit(const FCustomDamageEvent& DamageEvent)
{
	if(!IsValid(GetHitAnimation) || !AcceptedInputs.CanOverrideCurrentInput(EInputType::Stagger)) return;
	StopAnimMontage();
	PlayAnimMontage(GetHitAnimation);
	AcceptedInputs.LimitAvailableInputs({EInputType::Stagger, GetHitAnimation->GetPlayLength()*0.9f}, GetWorld());
}

void AFighterCharacter::OnDeath(const FCustomDamageEvent& DamageEvent)
{
	if(!IsValid(DeathAnimation) || !AcceptedInputs.CanOverrideCurrentInput(EInputType::Death)) return;
	StopAnimMontage();
	PlayAnimMontage(DeathAnimation);
	AcceptedInputs.LimitAvailableInputs({EInputType::Death, DeathAnimation->GetPlayLength()*0.9f}, GetWorld());
	AcceptedInputs.OnInputLimitsReset.AddLambda([this](bool IsLimitDurationOver, bool& HasBeenCleared)
	{
		HasBeenCleared = true;
		Destroy();
	});
}
