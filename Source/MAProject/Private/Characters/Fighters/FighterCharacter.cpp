// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/FighterCharacter.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/NonPlayerFunctionality/TargetInformationComponent.h"
#include "Utility/Sound/SoundResponseConfigs.h"

AFighterCharacter::AFighterCharacter() : bIsInvincible(false),  HitFXRadius(50.f)
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
	if(DamageEvent.IsOfType(FCustomDamageEvent::ClassID) && !bIsInvincible)
	{
		FDamageEvent* Event = const_cast<FDamageEvent*>(&DamageEvent);
		if(DamageEvent.IsOfType(FAttackDamageEvent::ClassID))
		{
			const FAttackDamageEvent AttackDamageEvent = *static_cast<FAttackDamageEvent*>(Event);
			RemainingHealth = CharacterStats->ReceiveDamage(DamageAmount, AttackDamageEvent);

			//Make hit sound which is defined per bone (if nothing is set for the bone, it will use the same sound as it's parent)
			check(IsValid(BoneSoundResponseConfig.Get()));
			FName BoneToCheck = GetMesh()->FindClosestBone(AttackDamageEvent.HitLocation);
			
			while(!BoneToCheck.IsNone())
			{
				FSoundConfig SoundConfig =
					BoneSoundResponseConfig.GetDefaultObject()->GetBoneResponses().FindRef(BoneToCheck);
				if(!(SoundConfig == FSoundConfig()))
				{
					SoundConfig.PlaySoundAtLocation(GetWorld(), AttackDamageEvent.HitLocation);
					break;
				}
				//there was no matching configuration found, so we'll look at the parent bone
				BoneToCheck = GetMesh()->GetParentBone(BoneToCheck);				
			}

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

void AFighterCharacter::StopAnimMontage(UAnimMontage* AnimMontage)
{
	if(IsValid(GetCurrentMontage()))
	{
		//Prevent melee enabled bones from not being disabled when the animation ends
		RecentlyDamagedActors.Empty();
		MeleeEnabledBones.Empty();
	}
	Super::StopAnimMontage(AnimMontage);
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

void AFighterCharacter::MakeInvincible(float InvincibilityTime)
{
	bIsInvincible = true;	
	if(GetWorld()->GetTimerManager().TimerExists(InvincibilityHandle))
		GetWorld()->GetTimerManager().ClearTimer(InvincibilityHandle);
	
	GetWorld()->GetTimerManager().SetTimer(InvincibilityHandle,
		[this]{ EndInvincibility(); }, InvincibilityTime, false);
}

void AFighterCharacter::EndInvincibility()
{
	bIsInvincible = false;
}

void AFighterCharacter::CheckMeshOverlaps()
{
	TArray<AActor*> OverlappingActors;
	GetMesh()->GetOverlappingActors(OverlappingActors);
	
	//loop through all bones that can damage targets, to find some that actually hit a target
	FHitResult TraceResult;
	for(const FName BodyName : MeleeEnabledBones)
	{
		FVector VelocityDirection = GetMesh()->GetBoneLinearVelocity(BodyName).GetSafeNormal();

		//we need hit results for attack management
		UKismetSystemLibrary::LineTraceSingle(GetWorld(), GetMesh()->GetBoneLocation(BodyName),
		GetMesh()->GetBoneLocation(BodyName) + VelocityDirection * 100.f, UEngineTypes::ConvertToTraceType(ECC_Destructible),
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
	const int32 Index = CurrentLimitIndex + 1;
	if(!InputLimits.IsValidIndex(Index)) return;
	AcceptedInputs.OnInputLimitsReset.AddWeakLambda(this,
		[=]
		(bool IsLimitDurationOver, bool& HasBeenCleared)
	{
		AcceptedInputs.LimitAvailableInputs(InputLimits[Index], GetWorld());
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
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AcceptedInputs.LimitAvailableInputs({EInputType::Death, DeathAnimation->GetPlayLength()*0.9f}, GetWorld());
	AcceptedInputs.OnInputLimitsReset.AddWeakLambda(this,
		[this](bool IsLimitDurationOver, bool& HasBeenCleared)
	{
		Destroy();
	});
}
