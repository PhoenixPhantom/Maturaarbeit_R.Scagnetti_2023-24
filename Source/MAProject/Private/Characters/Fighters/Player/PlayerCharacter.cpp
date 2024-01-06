// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Player/PlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Characters/Fighters/Attacks/AttackTree/AttackNode.h"
#include "Components/CapsuleComponent.h"
#include "Components/Image.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "UserInterface/StatsMonitorBaseWidget.h"
#include "UserInterface/HUD/Playerscreen/PlayerStatsMonitorBaseWidget.h"
#include "Utility/Animation/CustomAnimInstance.h"
#include "Utility/Animation/SuckToTargetComponent.h"
#include "Utility/NonPlayerFunctionality/TargetInformationComponent.h"
#include "Utility/Stats/StatusEffect.h"


FStoredInput::FStoredInput() : Timestamp(-1.0), ActionType(EInputType::Undefined)
{
}

FStoredInput::FStoredInput(double CurrentTime, EInputType AttemptedActionType, const TDelegate<void()>& AttemptedAction) :
	Timestamp(CurrentTime), ActionType(AttemptedActionType), RequestedAction(AttemptedAction)
{
}

void FStoredInput::TryUpdateStoredInput(double CurrentTime, double MaxInputWindow,
                                        EInputType AttemptedActionType, const TFunction<void()>& AttemptedAction)
{
	if(CurrentTime - Timestamp <= MaxInputWindow && IsValid())
	{
		//walk and camera cannot override the more important inputs
		switch(AttemptedActionType)
		{
			case EInputType::Walk: return;
			case EInputType::Camera: return;
			default: break;
		}
	}
	Timestamp = CurrentTime;
	ActionType = AttemptedActionType;
	RequestedAction.BindLambda(AttemptedAction);
}

void FStoredInput::Invalidate()
{
	RequestedAction.Unbind();
	ActionType = EInputType::Undefined;
}

bool FStoredInput::operator==(const FStoredInput& SavedInput) const
{
	return Timestamp == SavedInput.Timestamp && ActionType == SavedInput.ActionType &&
		RequestedAction.GetHandle() == SavedInput.RequestedAction.GetHandle();
}

APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer),
	bIsRunning(false), bHasJumped(false), bIsRestoringHealth(false), RestoreHealthTimestamp(0.0),
	CurrentTarget(nullptr), PlayerStatsMonitor(nullptr), AutotargetingRange(1000.f), DashOrBlinkCooldown(1.f),
	RememberInputDirectionTime(0.5), MaximalInputWindowTime(0.5)
{
	// Create a camera boom (pulls in towards the player if there is a collision)
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	SpringArm->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	HorizontalCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("HorizontalCapsuleComp"));
	HorizontalCapsule->SetupAttachment(GetCapsuleComponent());
	HorizontalCapsule->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	HorizontalCapsule->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
	HorizontalCapsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	HorizontalCapsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	HorizontalCapsule->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Block);
	HorizontalCapsule->SetGenerateOverlapEvents(false);

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	// Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 350.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
}

void APlayerCharacter::Tick(float DeltaSeconds)
{
	UpdateTargetSelection();
	if(bIsRestoringHealth)
	{
		if(GetWorld()->RealTimeSeconds - RestoreHealthTimestamp > 5.f)
		{
			CharacterStats->ChangeHealthByPercentage(+1.f);
			RestoreHealthTimestamp = GetWorld()->RealTimeSeconds;
		}
	}
	Super::Tick(DeltaSeconds);
}

void APlayerCharacter::GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	OutLocation = FollowCamera->GetComponentLocation();
	OutRotation = FollowCamera->GetComponentRotation();
}

float APlayerCharacter::GetFieldOfView() const
{
	return FollowCamera->FieldOfView;
}

void APlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if(IsValid(StatsMonitorWidget))
	{
		PlayerStatsMonitor->RemoveFromParent();
	}
}

void APlayerCharacter::PreSpawnSetup(FCharacterStats* PropertiesSource, FPlayerUserSettings* PlayerUserSettingsSource,
                                     const TDelegate<void(const FVector2D&)>& RequestedActionOnPlayerMovedCamera, FGenericTeamId NewTeamId, FPreSpawnSetupKey Key)
{
	OnPlayerMovedCamera = RequestedActionOnPlayerMovedCamera;
	InternalTeamId = NewTeamId;
	CharacterStats = PropertiesSource;
	PlayerUserSettings = PlayerUserSettingsSource;
}

float APlayerCharacter::RequestActionRank(const AActor* RankGenerationTarget) const
{
	//whether the target is on screen
	FVector EyesLocation;
	FRotator EyesRotation;
	GetActorEyesViewPoint(EyesLocation, EyesRotation);
	//Get the actors center
	FVector ActorCenter;
	FVector Extent;
	RankGenerationTarget->GetActorBounds(true, ActorCenter, Extent);

	
	float ActionRank = 0.f;
	const float OffsetFromForward = FVector::DotProduct(EyesRotation.Vector(),
	                                                    UKismetMathLibrary::GetDirectionUnitVector(
		                                                    EyesLocation, ActorCenter));
	if (UKismetMathLibrary::DegAcos(OffsetFromForward) <= GetFieldOfView() / 2.f) ActionRank += 1.5f;
	ActionRank += 1.f + OffsetFromForward; //there should be no negative action ranks
	
	//the current target should act whenever possible
	if(GetCurrentTarget() == RankGenerationTarget) ActionRank += BIG_NUMBER;
	return ActionRank;
}

AActor* APlayerCharacter::GetCurrentTarget() const
{
	return IsValid(CurrentTarget) ? CurrentTarget->GetOwner() : nullptr;
}


void APlayerCharacter::BeginPlay()
{
	check(IsValid(HealthWidgetClass.Get()));
	PlayerStatsMonitor = CreateWidget<UPlayerStatsMonitorBaseWidget>(GetWorld(), HealthWidgetClass);
	PlayerStatsMonitor->AddToViewport();
	RegisterHealthInfoWidget(PlayerStatsMonitor);
	
	Super::BeginPlay();
	OnAttackInterrupted.BindUObject(this, &APlayerCharacter::AttackInterrupted);
	CharacterStats->Attacks.OnExecuteAttack.AddDynamic(this, &APlayerCharacter::OnSelectMotionWarpingTarget);
	CharacterStats->Attacks.OnCdChanged.BindUObject(this, &APlayerCharacter::OnCdSet);
	if(IsValid(GetHitAnimation)) ToughnessBrokenTime = GetHitAnimation->GetPlayLength();
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	//Add Input Mapping Context
	if (const APlayerController* PlayerController = Cast<APlayerController>(Controller); IsValid(PlayerController))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
			IsValid(Subsystem))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	//Character specific input
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &APlayerCharacter::TryJump);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &APlayerCharacter::EndJump);

	EnhancedInputComponent->BindAction(LightAttackAction, ETriggerEvent::Triggered, this,
	                                   &APlayerCharacter::LightAttack);
	EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Triggered, this,
	                                   &APlayerCharacter::HeavyAttack);
	EnhancedInputComponent->BindAction(SkillAction, ETriggerEvent::Triggered, this, &APlayerCharacter::SkillAttack);
	EnhancedInputComponent->BindAction(UltimateAction, ETriggerEvent::Triggered, this,
	                                   &APlayerCharacter::UltimateAttack);

	EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Triggered, this, &APlayerCharacter::DashStartRunning);
	EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopRunning);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);

	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
	EnhancedInputComponent->BindAction(CameraZoomAction, ETriggerEvent::Triggered, this, &APlayerCharacter::CameraZoom);
	EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Aim);

	//UI input
	EnhancedInputComponent->BindAction(PauseMenuAction, ETriggerEvent::Triggered, this,
	                                   &APlayerCharacter::OpenPauseMenu);
}

bool APlayerCharacter::TriggerToughnessBroken()
{
	if(!Super::TriggerToughnessBroken()) return false;
	PlayAnimMontage(GetHitAnimation);
	CharacterStats->ChangeHealth(-10.f);
	return true;
}

float APlayerCharacter::GetLegIKBlendWeight(const FVector& Velocity)
{
	//this implementation is smoother than the default one when rock-climbing
	return FMath::Min((GetWorld()->RealTimeSeconds - InputDirection.Key)/RememberInputDirectionTime, 1.f);
}

void APlayerCharacter::QueueFollowUpLimit(const TArray<FNewInputLimits>& InputLimits)
{
	Super::QueueFollowUpLimit(InputLimits);
	if (LastInput.IsValid() && GetWorld()->GetRealTimeSeconds() - LastInput.Timestamp <= MaximalInputWindowTime
		&& AcceptedInputs.IsAllowedInput(LastInput.ActionType))
	{
		// ReSharper disable once CppExpressionWithoutSideEffects
		LastInput.RequestedAction.ExecuteIfBound();
	}
}

void APlayerCharacter::GenerateDamageEvent(FAttackDamageEvent& AttackDamageEvent, const FHitResult& CausingHit)
{
	Super::GenerateDamageEvent(AttackDamageEvent, CausingHit);
	//only the player can trigger time dilation on attack hits
	//We use a delegate, as the effect always has to apply on both contestants
	AttackDamageEvent.OnHitRegistered.BindWeakLambda(this, [this, AttackDamageEvent,
		SpringArmLength = SpringArm->TargetArmLength](bool HasStaggered)
	{
		if(HasStaggered) UGameplayStatics::PlayWorldCameraShake(GetWorld(), InduceStaggerCameraShake,
			AttackDamageEvent.HitLocation,0.f, 500.f + SpringArmLength);
		AFighterCharacter::OnHitTimeDilation(HasStaggered);
	});
}

bool APlayerCharacter::TriggerDeath()
{
	//we have to use a fully parent independent implementation here
	//as the GeneralCharacter already binds the destroy function to the OnInputLimitsReset
	//this would cause the game to crash if executed on the player character, so we bind the ShowDeathMenu function instead
	if(!IsValid(CustomAnimInstance) || !AcceptedInputs.IsAllowedInput(EInputType::Death) ||
		GetMesh()->GetCollisionEnabled() == ECollisionEnabled::NoCollision) return false;
	CustomAnimInstance->TriggerDeath();
	
	AcceptedInputs.LimitAvailableInputs({EInputType::Death, CustomAnimInstance->GetDeathAnimTime()}, GetWorld());
	TDelegate<void(bool)> OnDeathDelegate;
	OnDeathDelegate.BindUObject(this, &APlayerCharacter::ShowDeathMenu);
	AcceptedInputs.OnInputLimitsReset.Add(OnDeathDelegate);
	
	MakeInvincible(0.f);
	TargetInformationComponent->SetCanBeTargeted(false, FSetCanBeTargetedKey());
	return true;
}

void APlayerCharacter::CharacterLanded()
{
	Super::CharacterLanded();
	//Resets any input limits imposed by a previous state (Flying, Jumping)
	AcceptedInputs.LimitAvailableInputs(EInputType::Reset, GetWorld());
}

void APlayerCharacter::CharacterInAir()
{
	Super::CharacterInAir();
	AcceptedInputs.LimitAvailableInputs({EInputType::Jump, 0.f}, GetWorld());
}

void APlayerCharacter::OnAttackTreeModeChanged(FString NewRoot)
{
	Super::OnAttackTreeModeChanged(NewRoot);
	const UAttackNode* SkillNode = CharacterStats->Attacks.GetFirstNodeMatchingIndex(EAttackType::AttackType_Skill);
	PlayerStatsMonitor->SetSkillTimer(SkillNode->GetTimerHandle());
	PlayerStatsMonitor->SetTotalSkillCdTime(SkillNode->GetAttackProperties().GetTotalCdTime());

	const UAttackNode* UltimateNode = CharacterStats->Attacks.GetFirstNodeMatchingIndex(EAttackType::AttackType_Ultimate);
	PlayerStatsMonitor->SetUltimateTimer(UltimateNode->GetTimerHandle());
	PlayerStatsMonitor->SetTotalUltimateCdTime(UltimateNode->GetAttackProperties().GetTotalCdTime());
}

void APlayerCharacter::OnNewStatusEffectReceived(UStatusEffect* StatusEffect)
{
	Super::OnNewStatusEffectReceived(StatusEffect);
	StatusEffect->BindImage(PlayerStatsMonitor->GetFirstAvailableImage(), FStatusEffectBindImageKey());
}

void APlayerCharacter::OnStatusEffectRemoved()
{
	Super::OnStatusEffectRemoved();
	TArray<UActorComponent*> StatusEffectACs;
	GetComponents(UStatusEffect::StaticClass(), StatusEffectACs);
	TArray<UStatusEffect*> StatusEffects;
	for(UActorComponent* ActorComponent : StatusEffectACs)
	{
		StatusEffects.Add(CastChecked<UStatusEffect>(ActorComponent));
	}

	//Reorder the images so they don't have gap in between them
	while(true)
	{
		const UImage* UnconnectedImage = PlayerStatsMonitor->GetFirstUnconnectedImage();
		if(!IsValid(UnconnectedImage)) return;
		for(UStatusEffect* StatusEffect : StatusEffects){
			if(!StatusEffect->IsBoundImage(UnconnectedImage)) continue;
			StatusEffect->BindImage(PlayerStatsMonitor->GetFirstAvailableImage(), FStatusEffectBindImageKey());
			break;
		}
	}
}

void APlayerCharacter::TryJump()
{
	if (!AcceptedInputs.IsAllowedInput(EInputType::Jump))
	{
		LastInput.TryUpdateStoredInput(GetWorld()->GetRealTimeSeconds(), MaximalInputWindowTime,
		                               EInputType::Jump, [this]{ TryJump(); });
		return;
	}

	LastInput.Invalidate();
	AcceptedInputs.ResetLimits(GetWorld()); //Force interrupt here. This makes jump interrupts have effect faster. 
	Jump();
}

void APlayerCharacter::EndJump()
{
	StopJumping();
}

void APlayerCharacter::LightAttack()
{
	if (!AcceptedInputs.IsAllowedInput(EInputType::Attack))
	{
		LastInput.TryUpdateStoredInput(GetWorld()->GetRealTimeSeconds(), MaximalInputWindowTime,
		                               EInputType::Attack, [this]{ LightAttack(); });
		return;
	}

	LastInput.Invalidate();
	if(CharacterStats->Attacks.ExecuteAttack(EAttackType::AttackType_Light, this, GetWorld()))
	{
		AcceptedInputs.OnInputLimitsReset.Add(OnAttackInterrupted);
	}
}

void APlayerCharacter::HeavyAttack()
{
	if (!AcceptedInputs.IsAllowedInput(EInputType::Attack))
	{
		LastInput.TryUpdateStoredInput(GetWorld()->GetRealTimeSeconds(), MaximalInputWindowTime,
									   EInputType::Attack, [this]{ HeavyAttack(); });
		return;
	}

	LastInput.Invalidate();
	if(CharacterStats->Attacks.ExecuteAttack(EAttackType::AttackType_Heavy, this, GetWorld())){
		AcceptedInputs.OnInputLimitsReset.Add(OnAttackInterrupted);
	}
}

void APlayerCharacter::SkillAttack()
{
	if (!AcceptedInputs.IsAllowedInput(EInputType::Attack))
	{
		LastInput.TryUpdateStoredInput(GetWorld()->GetRealTimeSeconds(), MaximalInputWindowTime,
		                               EInputType::Attack, [this]{ SkillAttack(); });
		return;
	}

	LastInput.Invalidate();
	if(CharacterStats->Attacks.ExecuteAttack(EAttackType::AttackType_Skill, this, GetWorld()))
	{
		AcceptedInputs.OnInputLimitsReset.Add(OnAttackInterrupted);
	}
}

void APlayerCharacter::UltimateAttack()
{
	if (!AcceptedInputs.IsAllowedInput(EInputType::Attack))
	{
		LastInput.TryUpdateStoredInput(GetWorld()->GetRealTimeSeconds(), MaximalInputWindowTime,
		                               EInputType::Attack, [this]{ UltimateAttack(); });
		return;
	}

	LastInput.Invalidate();
	if(CharacterStats->Attacks.ExecuteAttack(EAttackType::AttackType_Ultimate, this, GetWorld()))
	{
		AcceptedInputs.OnInputLimitsReset.Add(OnAttackInterrupted);
	}
}

bool APlayerCharacter::Blink()
{
	if(GetWorld()->RealTimeSeconds - DashOrBlinkTimestamp <= DashOrBlinkCooldown) return true;
	const FVector& CurrentTargetLocation = GetCurrentTarget()->GetActorLocation();
	const FVector& DeltaDistance = CurrentTargetLocation - GetActorLocation();
	const float MinimalDistance = GetCurrentTarget()->GetSimpleCollisionRadius() + GetSimpleCollisionRadius();
	
	FVector MovementDirection;
	float Distance;
	DeltaDistance.ToDirectionAndLength(MovementDirection, Distance);
	//do not blink when too far away from the enemy
	if(Distance - MinimalDistance > 100.f) return false;

	//determine the appropriate position for the blink to teleport to
	FHitResult TraceResult;
	const FVector& TargetLocation = CurrentTargetLocation + MovementDirection.GetSafeNormal()*MinimalDistance;
	UKismetSystemLibrary::LineTraceSingle(GetWorld(), TargetLocation + FVector(0.0, 0.0, 500.f),
		TargetLocation + FVector(0.0, 0.0, -500.f), ETraceTypeQuery::TraceTypeQuery3,
		true, {this}, EDrawDebugTrace::None, TraceResult, true);

	if(!TraceResult.bBlockingHit) return false;

	//reposition/teleport; only if it does work, the blink will happen
	const FVector& ResultingLocation = TraceResult.Location + FVector(0.0, 0.0, GetSimpleCollisionHalfHeight());
	if(!SetActorLocation(ResultingLocation, false,nullptr,
		ETeleportType::TeleportPhysics)) return false;
	
	MakeInvincible(0.5f);
	FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(ResultingLocation,
		GetCurrentTarget()->GetActorLocation());
	TargetRotation.Pitch = 0.0;
	SetActorRotation(TargetRotation);
	
	DashOrBlinkTimestamp = GetWorld()->RealTimeSeconds;
	bIsRunning = true;
	
	return true;
}

void APlayerCharacter::DashStartRunning()
{
	if (!AcceptedInputs.IsAllowedInput(EInputType::Sprint))
	{
		LastInput.TryUpdateStoredInput(GetWorld()->GetRealTimeSeconds(), MaximalInputWindowTime,
		                               EInputType::Sprint, [this]{ DashStartRunning(); });
		return;
	}
	LastInput.Invalidate();
	AcceptedInputs.ResetLimits(GetWorld()); //force interrupt

	//try to blink to the other side of the current enemy
	if(IsValid(GetCurrentTarget()) && !bIsRunning && Blink()) return;
		
	//the first seconds of run are a dash
	if (!bIsRunning)
	{
		if(GetWorld()->RealTimeSeconds - DashOrBlinkTimestamp > DashOrBlinkCooldown){
			//use the last known movement direction if possible, otherwise use the camera view direction as dash direction
			FVector Direction = InputDirection.Value;
			if (GetWorld()->RealTimeSeconds - InputDirection.Key > RememberInputDirectionTime)
			{
				Direction = GetControlRotation().Vector();
				Direction.Z = 0.f;
				if (!Direction.Normalize())
				{
					FVector OutLocation;
					FRotator OutRotation;
					GetActorEyesViewPoint(OutLocation, OutRotation);
					Direction = OutRotation.Vector();
					Direction.Z = 0;
					if (!Direction.Normalize()) Direction = FVector(0.f);
				}
			}

			//Launch character doesn't work here since it sets the player's movement state to "falling" which causes it to
			//play the falling animation.
			GetCharacterMovement()->Velocity += FVector(Direction * CharacterStats->GetDashSpeed()) +
				FVector::ZAxisVector * GetCharacterMovement()->Velocity.Z;


			GetWorld()->GetTimerManager().SetTimerForNextTick(
				[this, OldRotationRate = GetCharacterMovement()->RotationRate]
				{
					GetCharacterMovement()->RotationRate = OldRotationRate;
				});
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
			{
				GetCharacterMovement()->Velocity *= 0.6;
			}, 0.1f, false);
			
			GetCharacterMovement()->RotationRate = FRotator(0.f, -1.f, 0.f);

			MakeInvincible(0.5f);
			
			DashOrBlinkTimestamp = GetWorld()->RealTimeSeconds;
		}

		//Since the player has already been displaced after a dash or run, we can always allow it to walk afterwards
		AcceptedInputs.AddAllowedInputType(EInputType::Walk);
		bIsRunning = true;

		SwitchMovementToRun(FSetWalkOrRunKey());
	}

	//Prevent having two inputs (from wasd keys and mouse) at the same time
	FVector Direction = ConsumeMovementInputVector();
	if (Direction.IsNearlyZero()) Direction = FVector::YAxisVector;
	Move(Direction);
}

void APlayerCharacter::StopRunning()
{
	bIsRunning = false;
	SwitchMovementToWalk(FSetWalkOrRunKey());
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (IsValid(Controller))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		//Prevent having two inputs (from wasd keys and mouse) at the same time
		if (bIsRunning) ConsumeMovementInputVector();

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		InputDirection.Key = GetWorld()->RealTimeSeconds;
		InputDirection.Value = ForwardDirection * MovementVector.Y + RightDirection * MovementVector.X;
		InputDirection.Value.Normalize();
		if ((!bIsRunning && AcceptedInputs.IsAllowedInput(EInputType::Walk)) ||
			(bIsRunning && AcceptedInputs.IsAllowedInput(EInputType::Sprint)))
		{
			// add movement
			LastInput.Invalidate();
			AcceptedInputs.ResetLimits(GetWorld()); //force interrupt
			AddMovementInput(ForwardDirection, MovementVector.Y);
			AddMovementInput(RightDirection, MovementVector.X);
		}
		else
		{
			LastInput.TryUpdateStoredInput(GetWorld()->GetRealTimeSeconds(), MaximalInputWindowTime,
			                               EInputType::Walk, [this, Value]{ Move(Value); });
		}
	}
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	if (IsValid(Controller))
	{
		if (!AcceptedInputs.IsAllowedInput(EInputType::Camera))
		{
			LastInput.TryUpdateStoredInput(GetWorld()->GetRealTimeSeconds(), MaximalInputWindowTime,
			                               EInputType::Camera, [this, Value]{ Look(Value); });
			return;
		}
		LastInput.Invalidate();
		const FVector2D& LookAxisVector = Value.Get<FVector2D>();
		// ReSharper disable once CppExpressionWithoutSideEffects
		OnPlayerMovedCamera.ExecuteIfBound(LookAxisVector);
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APlayerCharacter::CameraZoom(const FInputActionValue& Value)
{
	const float Movement = Value.Get<float>();
	if (AcceptedInputs.IsAllowedInput(EInputType::Camera))
		SpringArm->TargetArmLength += Movement * PlayerUserSettings->CameraZoomSpeed;
}

void APlayerCharacter::Aim(const FInputActionValue& Value)
{
	if (AcceptedInputs.IsAllowedInput(EInputType::Camera)) unimplemented();
}

void APlayerCharacter::OpenPauseMenu()
{
	UGameplayStatics::SetGamePaused(GetWorld(), true);
	UUserWidget* PauseMenu = CreateWidget<UUserWidget>(GetWorld(), PauseMenuClass);
	PauseMenu->AddToViewport();

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(PauseMenu->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	APlayerController* PlayerController = CastChecked<APlayerController>(GetController());
	PlayerController->SetInputMode(InputMode);
	PlayerController->SetShowMouseCursor(true);
}

void APlayerCharacter::UpdateTargetSelection()
{
	//get the player's view direction
	FVector EyesLocation;
	FRotator EyesRotation;
	GetActorEyesViewPoint(EyesLocation, EyesRotation);
	const FVector& PlayerLocation = GetActorLocation();

	//get the target (if any exists) that is right at the center of the player's vision
	FHitResult CenteredHitResult;
	UKismetSystemLibrary::LineTraceSingle(GetWorld(), EyesLocation,
										  EyesLocation + EyesRotation.Vector() * AutotargetingRange,
										  UEngineTypes::ConvertToTraceType(ECC_Destructible), true,
										  {this, Owner}, EDrawDebugTrace::None,
										  CenteredHitResult, true);
	
	TArray<FHitResult> TraceResults;
	UKismetSystemLibrary::SphereTraceMulti(GetWorld(), GetActorLocation(), GetActorLocation(),
		AutotargetingRange, UEngineTypes::ConvertToTraceType(ECC_Destructible),true,
		{this, Owner}, EDrawDebugTrace::None, TraceResults, true);

	TTuple<float, UTargetInformationComponent*> BestResult;
	BestResult.Key = std::numeric_limits<float>::lowest();
	BestResult.Value = nullptr;
	for (FHitResult TraceResult : TraceResults)
	{
		if (!TraceResult.bBlockingHit) continue; //all relevant meshes are set to block destructible objects...
		UActorComponent* Component = TraceResult.GetActor()->GetComponentByClass(UTargetInformationComponent::StaticClass());
		if (!IsValid(Component)) continue; //... and have a target information component
		UTargetInformationComponent* TargetInfoComp = CastChecked<UTargetInformationComponent>(Component);
		if(!TargetInfoComp->GetCanBeTargeted()) continue;
		
		//Get the actors center
		FVector ActorCenter;
		FVector Extent;
		TraceResult.GetActor()->GetActorBounds(true, ActorCenter, Extent);

		//whether the target is on screen
		float OffsetFromForward = FVector::DotProduct(EyesRotation.Vector(),
		        UKismetMathLibrary::GetDirectionUnitVector(EyesLocation, ActorCenter));
		if (UKismetMathLibrary::DegAcos(OffsetFromForward) > GetFieldOfView() / 2.f) continue;

		//the TargetInfoComp has to be visible to the camera...
		if(IsOccluded(UEngineTypes::ConvertToTraceType(ECC_Visibility),
			EyesLocation, ActorCenter, Extent, TraceResult.GetActor())) continue;
		//and to the actual character
		if(IsOccluded(UEngineTypes::ConvertToTraceType(ECC_Visibility),
			PlayerLocation, ActorCenter, Extent, TraceResult.GetActor())) continue;


		//Generate a score for the target priority
		float TotalScore = 0.f;
		TotalScore += 0.75f * OffsetFromForward; //together with centered actor we can still reach 1.f
		if (CenteredHitResult.bBlockingHit && CenteredHitResult.GetActor() == TraceResult.GetActor())
			TotalScore += 0.25f;
		if (GetWorld()->RealTimeSeconds - InputDirection.Key <= RememberInputDirectionTime)
			TotalScore += 3.f * FVector::DotProduct(InputDirection.Value,
			    UKismetMathLibrary::GetDirectionUnitVector(PlayerLocation, ActorCenter));
		TotalScore += 1.f - FVector::Distance(PlayerLocation, ActorCenter) / AutotargetingRange;
		if (TargetInfoComp->IsTargetOf(GetController())) TotalScore += 0.5f;
		TotalScore *= TargetInfoComp->GetTargetPriority();

		//only keep the best score
		if (TotalScore > BestResult.Key)
		{
			BestResult.Key = TotalScore;
			BestResult.Value = TargetInfoComp;
		}
	}

	//tell the target components who the new target is
	if (CurrentTarget != BestResult.Value)
	{
		if (IsValid(CurrentTarget)) CurrentTarget->RemoveTargetingEntity(GetController(), FSetTargetStateKey());
		if (IsValid(BestResult.Value))
		{
			CurrentTarget = BestResult.Value;
			CurrentTarget->AddTargetingEntity(GetController(), FSetTargetStateKey());
		}
		else CurrentTarget = nullptr;
	}

#if WITH_EDITORONLY_DATA
	//Draw debugging information
	if (bIsDebugging)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), AutotargetingRange, 100, FColor(0, 255, 0));
		if (GetWorld()->RealTimeSeconds - InputDirection.Key <= RememberInputDirectionTime)
			DrawDebugDirectionalArrow(GetWorld(),
			                          GetActorLocation(), GetActorLocation() + InputDirection.Value * 100.f,
			                          50.f, FColor(0, 0, 255), false, -1.f, 0, 5.f);
		if (IsValid(CurrentTarget))
			DrawDebugSphere(GetWorld(), CurrentTarget->GetComponentLocation(), 50.f,
			                20, FColor(100, 255, 100));
	}
#endif
}

bool APlayerCharacter::IsOccluded(ETraceTypeQuery TraceType, const FVector& ObserverLocation,
	const FVector& TargetCenter, const FVector& TargetExtent, AActor* TargetActor) const
{
	FHitResult VisibilityTrace;
	UKismetSystemLibrary::LineTraceSingle(GetWorld(), ObserverLocation, TargetCenter,
		TraceType, true,{const_cast<APlayerCharacter*>(this),
			Owner}, EDrawDebugTrace::None,VisibilityTrace, true);
	
	if (VisibilityTrace.bBlockingHit && VisibilityTrace.GetActor() != TargetActor)
	{
		//If the first probe didn't return visible, it doesn't mean the actor is really occluded
		TArray Locations({
			TargetCenter + TargetExtent,
			TargetCenter - TargetExtent,
			TargetCenter + FVector(TargetExtent.X, TargetExtent.Y, -TargetExtent.Z),
			TargetCenter + FVector(TargetExtent.X, -TargetExtent.Y, TargetExtent.Z),
			TargetCenter + FVector(-TargetExtent.X, TargetExtent.Y, TargetExtent.Z),
			TargetCenter - FVector(TargetExtent.X, TargetExtent.Y, -TargetExtent.Z),
			TargetCenter - FVector(TargetExtent.X, -TargetExtent.Y, TargetExtent.Z),
			TargetCenter - FVector(-TargetExtent.X, TargetExtent.Y, TargetExtent.Z)
		});
		return !AreMultipleVisible(TargetActor, TraceType, ObserverLocation, Locations, 3);
	}
	return false;
}

void APlayerCharacter::ShowDeathMenu(bool IsLimitDurationOver)
{
	UGameplayStatics::SetGamePaused(GetWorld(), true);
	UUserWidget* DeathMenu = CreateWidget<UUserWidget>(GetWorld(), DeathMenuClass);
	check(IsValid(DeathMenu));
	DeathMenu->AddToViewport();

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(DeathMenu->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	APlayerController* PlayerController = CastChecked<APlayerController>(GetController());
	PlayerController->SetInputMode(InputMode);
	PlayerController->SetShowMouseCursor(true);
}

void APlayerCharacter::OnCdSet(UAttackNode* IdentifiedNode, int32 Index)
{
	if(Index == EAttackType::AttackType_Skill)
	{
		PlayerStatsMonitor->SetSkillTimer(IdentifiedNode->GetTimerHandle());
	}
	else if(Index == EAttackType::AttackType_Ultimate)
	{
		PlayerStatsMonitor->SetUltimateTimer(IdentifiedNode->GetTimerHandle());		
	}
}

void APlayerCharacter::AttackInterrupted(bool IsLimitDurationOver)
{
	if(IsLimitDurationOver) return;
	StopAnimMontage();
	SuckToTargetComponent->InterruptWarping(FInterruptMotionWarpingKey());
}

void APlayerCharacter::OnSelectMotionWarpingTarget(const FAttackProperties& Properties)
{
	if (IsValid(CurrentTarget))
	{
		const FVector DeltaLocation = CurrentTarget->GetComponentLocation() - GetActorLocation();
		FVector Direction;
		float Length;
		DeltaLocation.ToDirectionAndLength(Direction, Length);
		FWarpInformation WarpInformation;
		if (Length <= Properties.MaximalMovementDistance)
		{
			WarpInformation.WarpSource = EWarpSource::FaceTargetObject;
			WarpInformation.TargetObject = CurrentTarget;
			WarpInformation.bFollowTarget = true;
			WarpInformation.MaxWarpingDistance = Properties.MaximalMovementDistance;
		}
		else
		{
			WarpInformation.WarpSource = EWarpSource::FaceLocation;
			WarpInformation.TargetLocation = GetActorLocation() + Direction * Properties.DefaultMovementDistance;
		}
		SuckToTargetComponent->SetOrUpdateWarpTarget(WarpInformation);
	}
	else
	{
		FVector AttackPosition;
		if (GetWorld()->RealTimeSeconds - InputDirection.Key > RememberInputDirectionTime)
		{
			FVector PlayerLocation;
			FRotator PlayerRotation;
			GetActorEyesViewPoint(PlayerLocation, PlayerRotation);
			FVector PlayerForward = PlayerRotation.Vector();
			PlayerForward.Z = 0.f;
			if (!PlayerForward.Normalize())
			{
				Super::GetActorEyesViewPoint(PlayerLocation, PlayerRotation);
				PlayerForward = PlayerRotation.Vector();
			}
			AttackPosition = GetActorLocation() + PlayerForward * Properties.DefaultMovementDistance / 2.f;
		}
		else AttackPosition = GetActorLocation() + InputDirection.Value * Properties.DefaultMovementDistance / 2.f;
		FWarpInformation WarpInformation;
		WarpInformation.WarpSource = EWarpSource::FaceLocation;
		WarpInformation.TargetLocation = AttackPosition;
		SuckToTargetComponent->SetOrUpdateWarpTarget(WarpInformation);
	}
}
