// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/FighterCharacter.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Damage.h"
#include "UserInterface/StatsMonitorBaseWidget.h"
#include "Utility/NonPlayerFunctionality/TargetInformationComponent.h"
#include "Utility/Sound/SoundResponseConfigs.h"


AFighterCharacter::AFighterCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer),
	bIsInvincible(false),  TargetTimeDilation(-1.f), TimeDilationBlendTime(-1.f), TimeDilationTotalTime(-1.f),
	TimeDilationEffectTimeRemaining(-1.f), CharacterStats(nullptr), ToughnessBrokenTime(1.f), HitFXRadius(50.f)
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
	int32 RemainingHealth = CharacterStats->Health.Current;
	if(
		FGenericTeamId::GetAttitude(this, DamageCauser) != ETeamAttitude::Friendly && !bIsInvincible &&
		DamageEvent.IsOfType(FCustomDamageEvent::ClassID))
	{
		const FCustomDamageEvent* Event = static_cast<const FCustomDamageEvent*>(&DamageEvent);
		if(DamageEvent.IsOfType(FAttackDamageEvent::ClassID))
		{
			const FAttackDamageEvent* AttackDamageEvent = static_cast<const FAttackDamageEvent*>(Event);
			RemainingHealth = CharacterStats->ReceiveDamage(DamageAmount, AttackDamageEvent);
			
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
	if(!CharacterStats->Attacks.HasPendingAttackProperties()) return;
	if(StartEmpty) MeleeEnabledBones.Empty();
	if(AllowHitRecentVictims) RecentlyDamagedActors.Empty();
	MeleeEnabledBones.Append(BonesToEnable);
}

void AFighterCharacter::DeactivateMeleeBones(const TArray<FName>& BonesToDisable, bool IsLastAttackOfAnimation,
	FMeleeControlsKey Key)
{
	if(IsLastAttackOfAnimation)
	{
		RecentlyDamagedActors.Empty();
		CharacterStats->Attacks.ClearPendingAttackPropertiesInternal();
	}
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
	FHitResult HitTraceResult;
	for(const FName BodyName : MeleeEnabledBones)
	{
		FVector VelocityDirection = GetMesh()->GetBoneLinearVelocity(BodyName).GetSafeNormal();

		//we need hit results for attack management
		UKismetSystemLibrary::LineTraceSingle(GetWorld(), GetMesh()->GetBoneLocation(BodyName),
		GetMesh()->GetBoneLocation(BodyName) + VelocityDirection * 100.f, UEngineTypes::ConvertToTraceType(ECC_Destructible),
		true, {this, Owner}, EDrawDebugTrace::None, HitTraceResult, true);

		if(HitTraceResult.bBlockingHit)
		{
			for(AActor* Target : OverlappingActors)
			{
				if(Target == this || !IsValid(Target)  || RecentlyDamagedActors.Contains(Target) ||
					HitTraceResult.GetActor() != Target) continue;
				RecentlyDamagedActors.Add(Target);
				FAttackDamageEvent AttackDamageEvent;
				GenerateDamageEvent(AttackDamageEvent, HitTraceResult);
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

void AFighterCharacter::GenerateDamageEvent(FAttackDamageEvent& AttackDamageEvent,
	const FHitResult& CausingHit)
{
	CharacterStats->GenerateDamageEvent(AttackDamageEvent, CausingHit);
}

bool AFighterCharacter::TriggerDeath()
{
	if(!Super::TriggerDeath()) return false;
	MakeInvincible(0.f);
	TargetInformationComponent->SetCanBeTargeted(false, FSetCanBeTargetedKey());
	return true;
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

void AFighterCharacter::GetStaggered(bool HeavyStagger)
{
	check(IsValid(GetHitAnimation));
	EInputType StaggerType = EInputType::Stagger;
	if(HeavyStagger) StaggerType = EInputType::HeavyStagger;
	if(!IsValid(GetHitAnimation) || !AcceptedInputs.IsAllowedInput(StaggerType)) return;

	PlayAnimMontage(GetHitAnimation);
	AcceptedInputs.LimitAvailableInputs({StaggerType, GetHitAnimation->GetPlayLength() - 0.1f}, GetWorld());
}

void AFighterCharacter::OnGetDamaged(const FCustomDamageEvent* DamageEvent)
{
	//Damaged by an attack
	if(DamageEvent->IsOfType(FAttackDamageEvent::ClassID))
	{
		GLog->Log("the function got called and correctly");
		OnGetAttacked(static_cast<const FAttackDamageEvent*>(DamageEvent));
	}
	//Damaged by something else
	else
	{
		GLog->Log("the function got called but wrongly");
	}	
}

bool AFighterCharacter::TriggerToughnessBroken()
{
	check(ToughnessBrokenTime > 0.f);
	if(!AcceptedInputs.LimitAvailableInputs({EInputType::HeavyStagger, ToughnessBrokenTime}, GetWorld())) return false;

	FCharacterStatsBuffs Debuff;
	Debuff.DefenseBuff = -50.f;
	TDelegate<void(bool)> OnToughnessBrokenReset;
	OnToughnessBrokenReset.BindWeakLambda(this, [this, Debuff](bool IsLimitDurationOver)
	{
		if(!IsLimitDurationOver)
		{
			GLog->Log(GetActorNameOrLabel() + " seems to have died while while it's toughness was broken");
		}
		RestoreToughness();
		ApplyBuff(Debuff.ReverseCharacterStatsBuffs());
	});
	AcceptedInputs.OnInputLimitsReset.Add(OnToughnessBrokenReset);
	ApplyBuff(Debuff);
	return true;
}

void AFighterCharacter::RestoreToughness()
{
	CharacterStats->RefillToughness();
}

void AFighterCharacter::OnGetAttacked(const FAttackDamageEvent* DamageEvent)
{
	PlayHitSound(DamageEvent->HitLocation);
	SpawnHitFX(DamageEvent->HitLocation, DamageEvent->HitFXScaleFactor);
		
	// ReSharper disable once CppExpressionWithoutSideEffects
	DamageEvent->OnHitRegistered.ExecuteIfBound(false);
	OnHitTimeDilation(false);
}

void AFighterCharacter::QueueFollowUpLimit(const TArray<FNewInputLimits>& InputLimits)
{
	if(InputLimits.IsEmpty()) return;
	TDelegate<void(bool)> FollowUpWithLimit;
	FollowUpWithLimit.BindWeakLambda(this,[this, InputLimits](bool IsLimitDurationOver)
		{
			if(!IsLimitDurationOver) return;
			AcceptedInputs.LimitAvailableInputs(InputLimits[0], GetWorld());			
			TArray NewLimits(InputLimits);
			NewLimits.RemoveAt(0);
			QueueFollowUpLimit(NewLimits);
		});
	AcceptedInputs.OnInputLimitsReset.Add(FollowUpWithLimit);
}

void AFighterCharacter::OnHitTimeDilation(bool WasStaggered)
{
	if(WasStaggered) BlendTimeDilation(0.175f, 0.35f, 0.f);
	else BlendTimeDilation(0.15f, 0.3f, 0.15f);
}

void AFighterCharacter::RegisterHealthInfoWidget(UStatsMonitorBaseWidget* Widget)
{
	check(IsValid(Widget));
	StatsMonitorWidget = Widget;

	StatsMonitorWidget->SetupInformation(CharacterStats->Health, CharacterStats->Toughness, FSetupInformationKey());
	
	CharacterStats->OnHealthChanged.AddDynamic(StatsMonitorWidget, &UStatsMonitorBaseWidget::UpdateHealth);
	CharacterStats->OnMaxHealthChanged.AddDynamic(StatsMonitorWidget, &UStatsMonitorBaseWidget::UpdateMaxHealth);
	CharacterStats->OnToughnessChanged.AddDynamic(StatsMonitorWidget, &UStatsMonitorBaseWidget::UpdateToughness);
	CharacterStats->OnMaxToughnessChanged.AddDynamic(StatsMonitorWidget, &UStatsMonitorBaseWidget::UpdateMaxToughness);
}

void AFighterCharacter::SetAttackTreeMode(FString ModeIdentifier)
{
	CharacterStats->Attacks.SetModeIdentifier(ModeIdentifier, FSetAttackTreeModeIdentifier());
	OnAttackTreeModeChanged(ModeIdentifier);
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
	CharacterStats->OnHealthChanged.AddDynamic(this, &AFighterCharacter::OnHealthChanged);
	CharacterStats->OnNoHealthReached.AddDynamic(this, &AFighterCharacter::OnDeath);
	CharacterStats->OnNoToughnessReached.AddDynamic(this, &AFighterCharacter::OnToughnessBroken);
	CharacterStats->Attacks.OnExecuteAttack.AddDynamic(this, &AFighterCharacter::OnExecuteAttack);
	CharacterStats->Attacks.OnCheckCanExecuteAttack.BindDynamic(this, &AFighterCharacter::OnCheckCanExecuteAttack);
	//This is a hack to allow passing a FCustomDamageEvent* without loss or errors
	CharacterStats->OnGetDamaged.BindWeakLambda(this, [this](const FCustomDamageEvent* DamageEvent)
		{ OnGetDamaged(DamageEvent); });
	SwitchMovementToWalk(FSetWalkOrRunKey());
	OnAttackTreeModeChanged("");
}

void AFighterCharacter::ApplyBuffTimed(const FCharacterStatsBuffs& Buffs, float Duration)
{
	if(Duration <= 0.f)
	{
		checkNoEntry();
		return;
	}
	
	CharacterStats->Buff(Buffs);
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [Buffs, Local = this]()
	{
		if(IsValid(Local)) Local->CharacterStats->Debuff(Buffs);
	}, Duration, false);	
}

void AFighterCharacter::ApplyBuff(const FCharacterStatsBuffs& Buffs)
{
	CharacterStats->Buff(Buffs);
}

void AFighterCharacter::RemoveBuff(bool bResetHealth, bool bResetAttack, bool bResetDefense, bool bResetToughness,
	bool bResetRunSpeed, bool bResetWalkSpeed, bool bResetInterruptionResistance)
{
	if(bResetHealth) CharacterStats->ResetHealth();
	if(bResetAttack) CharacterStats->ResetAttack();
	if(bResetDefense) CharacterStats->ResetDefense();
	if(bResetToughness) CharacterStats->ResetToughness();
	if(bResetRunSpeed) CharacterStats->ResetRunSpeed();
	if(bResetWalkSpeed) CharacterStats->ResetWalkSpeed();
	if(bResetInterruptionResistance) CharacterStats->ResetInterruptionResistance();
}

void AFighterCharacter::ForceSetCd(FString NodeName, float CdTime, bool ChangeBy)
{
	CharacterStats->Attacks.ForceSetCd(NodeName, CdTime, ChangeBy);
}

void AFighterCharacter::OnHealthChanged_Implementation(int32 NewHealth, int32 OldHealth)
{
	
}

bool AFighterCharacter::OnCheckCanExecuteAttack(const FAttackProperties& Properties)
{
	return AcceptedInputs.IsAllowedInput(Properties.InputLimits[0].LimiterType) && !GetCharacterMovement()->IsFalling();
}

void AFighterCharacter::OnExecuteAttack(const FAttackProperties& Properties)
{
	PlayAnimMontage(Properties.AtkAnimation);
	AcceptedInputs.LimitAvailableInputs(Properties.InputLimits[0], GetWorld());
	QueueFollowUpLimit(Properties.InputLimits);
}
