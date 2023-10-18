// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/FighterCharacter.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Damage.h"
#include "UserInterface/HealthMonitorBaseWidget.h"
#include "Utility/NonPlayerFunctionality/TargetInformationComponent.h"
#include "Utility/Sound/SoundResponseConfigs.h"


AFighterCharacter::AFighterCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer),
                                                                                    bIsInvincible(false),  TargetTimeDilation(-1.f), TimeDilationBlendTime(-1.f), TimeDilationTotalTime(-1.f),
                                                                                    TimeDilationEffectTimeRemaining(-1.f), HitFXRadius(50.f)
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
	if(TimeDilationEffectTimeRemaining > 0.f) ProcessTimeDilation(DeltaSeconds);
}

float AFighterCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
                                    AActor* DamageCauser)
{
	uint32 RemainingHealth = CharacterStats->Health;
	if(
		FGenericTeamId::GetAttitude(this, DamageCauser) != ETeamAttitude::Friendly && !bIsInvincible &&
		DamageEvent.IsOfType(FCustomDamageEvent::ClassID))
	{
		const FCustomDamageEvent* Event = static_cast<const FCustomDamageEvent*>(&DamageEvent);
		if(DamageEvent.IsOfType(FAttackDamageEvent::ClassID))
		{
			const FAttackDamageEvent* AttackDamageEvent = static_cast<const FAttackDamageEvent*>(Event);
			RemainingHealth = CharacterStats->ReceiveDamage(DamageAmount, AttackDamageEvent);

			//TODO: this is just a hack to test out hit stop
			if(EventInstigator == GetWorld()->GetFirstPlayerController())
			{
				BlendTimeDilation(0.15f, 0.3f, 0.15f);
				CastChecked<AFighterCharacter>(DamageCauser)->BlendTimeDilation(0.15f, 0.3f, 0.15f);
			}
			
			UAISense_Damage::ReportDamageEvent(GetWorld(), this, EventInstigator->GetPawn(), DamageAmount,
				EventInstigator->GetPawn()->GetActorLocation(), AttackDamageEvent->HitLocation);
		}
		else RemainingHealth = CharacterStats->FGeneralObjectStats::ReceiveDamage(DamageAmount,
			Event);
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

void AFighterCharacter::BlendTimeDilation(float BlendTime, float TotalTime, float TargetDilation)
{
	check(BlendTime * 2.f <= TotalTime && TargetDilation != 1.f);
	SourceTimeDilation = 1.f;
	TimeDilationBlendTime = BlendTime;
	TimeDilationTotalTime = TotalTime;
	TimeDilationEffectTimeRemaining = TotalTime;
	TargetTimeDilation = TargetDilation;
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
	for(FName BoneToDisable : BonesToDisable) MeleeEnabledBones.RemoveSwap(BoneToDisable);
}

void AFighterCharacter::AddOnInputLimitsResetDelegate(const TDelegate<void(bool)>& FunctionToAdd, FModifyInputLimitsKey)
{
	AcceptedInputs.OnInputLimitsReset.Add(FunctionToAdd);
}

void AFighterCharacter::RemoveOnInputLimitsResetDelegate(const TDelegate<void(bool)>& FunctionToAdd,
	FModifyInputLimitsKey)
{
	AcceptedInputs.OnInputLimitsReset.RemoveSwap(FunctionToAdd);
}


void AFighterCharacter::MakeInvincible(float InvincibilityTime)
{
	bIsInvincible = true;	
	if(GetWorld()->GetTimerManager().TimerExists(InvincibilityHandle))
		GetWorld()->GetTimerManager().ClearTimer(InvincibilityHandle);

	if(InvincibilityTime <= 0.f) return;
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
				FAttackDamageEvent AttackDamageEvent;
				CharacterStats->GenerateDamageEvent(AttackDamageEvent, TraceResult);
				Target->TakeDamage(CharacterStats->GetDamageOutput(),
					AttackDamageEvent, GetInstigatorController(), this);
				break;
			}
		}
	}
}

void AFighterCharacter::ProcessTimeDilation(float DeltaSeconds)
{
	TimeDilationEffectTimeRemaining -= DeltaSeconds / CustomTimeDilation; //we need the "real" time
	if(TimeDilationBlendTime >= (TimeDilationTotalTime - TimeDilationEffectTimeRemaining))
	{
		//blend to target
		const float InterpolationAmount = pow((TimeDilationTotalTime - TimeDilationEffectTimeRemaining) /
			TimeDilationBlendTime, 2.5);
		const float ResultingDilation = SourceTimeDilation * (1.f - InterpolationAmount) +
			TargetTimeDilation * InterpolationAmount;

		//prevent jumping around when blending is interrupted by another blending order
		if((SourceTimeDilation > TargetTimeDilation && ResultingDilation > CustomTimeDilation) ||
			(SourceTimeDilation < TargetTimeDilation && ResultingDilation < CustomTimeDilation))
				CustomTimeDilation = ResultingDilation;
	}
	else if(TimeDilationBlendTime >= TimeDilationEffectTimeRemaining)
	{
		if(TimeDilationEffectTimeRemaining <= 0.f)
		{
			//prevent overshooting
			CustomTimeDilation = SourceTimeDilation;
		}
		else
		{
			//blend to original
			const float InterpolationAmount = pow(TimeDilationEffectTimeRemaining / TimeDilationBlendTime, 2.5);
			const float ResultingDilation = SourceTimeDilation * InterpolationAmount +
				TargetTimeDilation * (1.f - InterpolationAmount);
			
			//prevent jumping around when blending is interrupted by another blending order
			if((SourceTimeDilation > TargetTimeDilation && ResultingDilation < CustomTimeDilation) ||
				(SourceTimeDilation < TargetTimeDilation && ResultingDilation > CustomTimeDilation))
					CustomTimeDilation = ResultingDilation;
		}
	}
	else
	{
		CustomTimeDilation = TargetTimeDilation; 
	}
}

void AFighterCharacter::OnDeathTriggered()
{
	TargetInformationComponent->SetCanBeTargeted(false, FSetCanBeTargetedKey());
}

void AFighterCharacter::PlayHitSound(const FVector& HitLocation)
{
	//Make hit sound which is defined per bone (if nothing is set for the specific given bone,
	//it is assumed to use the same sound as it's parent)
	check(IsValid(BoneSoundResponseConfig.Get()));
	FName BoneToCheck = GetMesh()->FindClosestBone(HitLocation);
			
	while(!BoneToCheck.IsNone())
	{
		FSoundConfig SoundConfig =
			BoneSoundResponseConfig.GetDefaultObject()->GetBoneResponses().FindRef(BoneToCheck);
		if(!(SoundConfig == FSoundConfig()))
		{
			SoundConfig.PlaySoundAtLocation(GetWorld(), HitLocation);
			break;
		}
		//there was no matching configuration found, so we'll look at the parent bone
		BoneToCheck = GetMesh()->GetParentBone(BoneToCheck);				
	}
}

void AFighterCharacter::SpawnHitFX(const FVector& Location, float ScaleFactor)
{
	//Spawn get hit FX
	constexpr float RealRadius = 75.f;
	FFXSystemSpawnParameters SpawnParameters;
	SpawnParameters.SystemTemplate = GetHitFX;
	SpawnParameters.Location = Location;
	SpawnParameters.Scale = FVector(HitFXRadius/RealRadius * ScaleFactor);
	SpawnParameters.AttachToComponent = GetRootComponent();
	SpawnParameters.bAutoActivate = true;
	SpawnParameters.bAutoDestroy = true;
	SpawnParameters.WorldContextObject = GetWorld();
	UNiagaraComponent* NiagaraComponent =
		UNiagaraFunctionLibrary::SpawnSystemAttachedWithParams(SpawnParameters);
	NiagaraComponent->SetVariableLinearColor("BaseColor", FLinearColor(0.5f, 0.5f, 0.5f));
}

void AFighterCharacter::GetStaggered(const FAttackDamageEvent* DamageEvent)
{
	check(IsValid(GetHitAnimation));
	if(!IsValid(GetHitAnimation) || !AcceptedInputs.CanOverrideCurrentInput(EInputType::Stagger)) return;
	
	StopAnimMontage();
	PlayAnimMontage(GetHitAnimation);
	AcceptedInputs.LimitAvailableInputs({EInputType::Stagger, GetHitAnimation->GetPlayLength()*0.9f}, GetWorld());
}

void AFighterCharacter::QueueFollowUpLimit(const TArray<FInputLimits>& InputLimits)
{
	if(InputLimits.IsEmpty()) return;
	TDelegate<void(bool)> FollowUpWithLimit;
	FollowUpWithLimit.BindWeakLambda(this,[this, InputLimits](bool IsLimitDurationOver)
		{
			AcceptedInputs.LimitAvailableInputs(InputLimits[0], GetWorld());			
			TArray NewLimits(InputLimits);
			NewLimits.RemoveAt(0);
			QueueFollowUpLimit(NewLimits);
		});
	AcceptedInputs.OnInputLimitsReset.Add(FollowUpWithLimit);
}

void AFighterCharacter::RegisterHealthInfoWidget(UHealthMonitorBaseWidget* Widget)
{
	check(IsValid(Widget));
	HealthInfoWidget = Widget;

	HealthInfoWidget->SetupInformation(CharacterStats->MaxHealth.GetResulting(),
	CharacterStats->MaxHealth.GetResulting(), FSetupInformationKey());
	
	CharacterStats->OnHealthChanged.AddDynamic(HealthInfoWidget, &UHealthMonitorBaseWidget::UpdateHealth);
}

void AFighterCharacter::SwitchMovementToWalk(FSetWalkOrRunKey) const
{
	GetCharacterMovement()->MaxWalkSpeed = CharacterStats->WalkSpeed.GetResulting();
}

void AFighterCharacter::SwitchMovementToRun(FSetWalkOrRunKey) const
{
	GetCharacterMovement()->MaxWalkSpeed = CharacterStats->RunSpeed.GetResulting();
}

void AFighterCharacter::BeginPlay()
{
	Super::BeginPlay();
	check(GetMesh()->GetRelativeTransform().GetMaximumAxisScale() == GetMesh()->GetRelativeTransform().GetMinimumAxisScale());
	SetAnimRootMotionTranslationScale(GetMesh()->GetRelativeTransform().GetMaximumAxisScale()/100.f);
	CharacterStats->Attacks.OnExecuteAttack.AddDynamic(this, &AFighterCharacter::OnExecuteAttack);
	CharacterStats->Attacks.OnCheckCanExecuteAttack.BindDynamic(this, &AFighterCharacter::OnCheckCanExecuteAttack);
#if USE_UE5_DELEGATE
	CharacterStats->OnGetDamaged.AddDynamic(this, &AFighterCharacter::OnGetDamagedUE);
#else
	//TODO: This is a hack to allow passing a FCustomDamageEvent* without loss or errors
	CharacterStats->OnGetDamaged.BindWeakLambda(this, [this](const FCustomDamageEvent* DamageEvent)
		{ OnGetDamaged(DamageEvent); });
#endif
	CharacterStats->OnNoHealthReached.AddDynamic(this, &AFighterCharacter::OnDeath);
	SwitchMovementToWalk(FSetWalkOrRunKey());
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

void AFighterCharacter::OnGetDamaged(const FCustomDamageEvent* DamageEvent)
{
	//Damaged by an attack
	if(DamageEvent->IsOfType(FAttackDamageEvent::ClassID))
	{
		GLog->Log("the function got called and correctly");
		const FAttackDamageEvent* AttackDamageEvent = static_cast<const FAttackDamageEvent*>(DamageEvent);
		PlayHitSound(AttackDamageEvent->HitLocation);
		SpawnHitFX(AttackDamageEvent->HitLocation, AttackDamageEvent->HitFXScaleFactor);
		
		const uint32 CasePerThousand = FMath::RandRange(0, 1000);
		if(CasePerThousand <= AttackDamageEvent->StaggerChance)
		{
			GetStaggered(AttackDamageEvent);
		}
	}
	//Damaged by something else
	else
	{
		GLog->Log("the function got called but wrongly");
	}	
}

void AFighterCharacter::OnDeath(const FCustomDamageEvent& DamageEvent)
{
	if(!IsValid(DeathAnimation) || !AcceptedInputs.CanOverrideCurrentInput(EInputType::Death) ||
		GetMesh()->GetCollisionEnabled() == ECollisionEnabled::NoCollision) return;
	OnDeathTriggered();
	StopAnimMontage();
	PlayAnimMontage(DeathAnimation);
	MakeInvincible(0.f);
	
	AcceptedInputs.LimitAvailableInputs({EInputType::Death, DeathAnimation->GetPlayLength()*0.9f}, GetWorld());
	TDelegate<void(bool)> OnDeathDelegate;
	OnDeathDelegate.BindWeakLambda(this, [this](bool IsLimitDurationOver)
	{
		Destroy();
	});
	AcceptedInputs.OnInputLimitsReset.Add(OnDeathDelegate);
}
