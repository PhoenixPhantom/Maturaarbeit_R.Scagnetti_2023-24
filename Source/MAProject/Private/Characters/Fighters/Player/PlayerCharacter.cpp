// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Player/PlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MotionWarpingComponent.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Utility/Animation/SuckToTargetComponent.h"
#include "Utility/NonPlayerFunctionality/TargetInformationComponent.h"

APlayerCharacter::APlayerCharacter() : bIsRunning(false), CurrentTarget(nullptr), AutotargetingRange(1000.f)
{
	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
}

void APlayerCharacter::


Tick(float DeltaSeconds)
{
	UpdateTargetSelection();
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

void APlayerCharacter::PreSpawnSetup(FCharacterStats* PropertiesSource, FPlayerUserSettings* PlayerUserSettingsSource,
                                     FPreSpawnSetupKey Key)
{
	CharacterStats = PropertiesSource;
	PlayerUserSettings = PlayerUserSettingsSource;
}


void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	CharacterStats->OnExecuteAttack.AddDynamic(this, &APlayerCharacter::OnSelectMotionWarpingTarget);
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	//Add Input Mapping Context
	if (const APlayerController* PlayerController = Cast<APlayerController>(Controller); IsValid(PlayerController))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()); IsValid(Subsystem))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	//Character specific input
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Jump);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopJumping);
	
	EnhancedInputComponent->BindAction(LightAttackAction, ETriggerEvent::Triggered, this, &APlayerCharacter::LightAttack);
	EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Triggered, this, &APlayerCharacter::HeavyAttack);
	EnhancedInputComponent->BindAction(SkillAction, ETriggerEvent::Triggered, this, &APlayerCharacter::SkillAttack);
	EnhancedInputComponent->BindAction(UltimateAction, ETriggerEvent::Triggered, this, &APlayerCharacter::UltimateAttack);

	EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Triggered,this, &APlayerCharacter::SpeedUpDash);
	EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Completed, this, &APlayerCharacter::SlowDown); 
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
	
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
	EnhancedInputComponent->BindAction(CameraZoomAction, ETriggerEvent::Triggered, this, &APlayerCharacter::CameraZoom);
	EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Aim);
	
	//UI input
	EnhancedInputComponent->BindAction(PauseMenuAction, ETriggerEvent::Triggered, this, &APlayerCharacter::OpenPauseMenu);
}

void APlayerCharacter::LightAttack(const FInputActionValue& Value)
{
	CharacterStats->ExecuteAttack(EAttackType::Light);
}

void APlayerCharacter::HeavyAttack(const FInputActionValue& Value)
{
	CharacterStats->ExecuteAttack(EAttackType::Heavy);
}

void APlayerCharacter::SkillAttack(const FInputActionValue& Value)
{
	CharacterStats->ExecuteAttack(EAttackType::Skill);
}

void APlayerCharacter::UltimateAttack(const FInputActionValue& Value)
{
	CharacterStats->ExecuteAttack(EAttackType::Ultimate);
}

void APlayerCharacter::SpeedUpDash(const FInputActionValue& Value)
{
	if(!AcceptedInputs.bCanSprint) return;
	//we only have (and should only) to set variables one time
	if(!bIsRunning)
	{
		SwitchMovementToRun();
	
		//Since on ongoing: dash will be called, we can assume that the player moves every time you dash, so
		//a dash will always allows you to walk after execution because you've already been displaced)
		AcceptedInputs.MovementProperties.bCanWalk = true;
		bIsRunning = true;
	}
	else
	{
		//Prevent having two inputs (from wasd keys and mouse) at the same time
		ConsumeMovementInputVector();
		Move(FVector2D(0.f, 1.f));
	}
}

void APlayerCharacter::SlowDown(const FInputActionValue& Value)
{
	bIsRunning = false;
	SwitchMovementToWalk();
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
		if(bIsRunning) ConsumeMovementInputVector();
		
		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		InputDirection.Key = GetWorld()->RealTimeSeconds;
		InputDirection.Value = ForwardDirection * MovementVector.Y + RightDirection * MovementVector.X;
		InputDirection.Value.Normalize();
		if(AcceptedInputs.MovementProperties.bCanWalk)
		{
			// add movement 
			AddMovementInput(ForwardDirection, MovementVector.Y);
			AddMovementInput(RightDirection, MovementVector.X);
		}
	}
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (IsValid(Controller) && AcceptedInputs.bFreeCameraAdjustment)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APlayerCharacter::CameraZoom(const FInputActionValue& Value)
{
	const float Movement = Value.Get<float>();
	if(AcceptedInputs.bFreeCameraAdjustment)
		CameraBoom->TargetArmLength += Movement * PlayerUserSettings->CameraZoomSpeed;
}

void APlayerCharacter::Aim(const FInputActionValue& Value)
{
	if(AcceptedInputs.bFreeCameraAdjustment) unimplemented();
}

void APlayerCharacter::OpenPauseMenu(const FInputActionValue& Value)
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
	TArray<FHitResult> TraceResults;
	//tracing on TraceTypeQuery6 (== Destructible)
	UKismetSystemLibrary::SphereTraceMulti(GetWorld(), GetActorLocation(), GetActorLocation(),
	AutotargetingRange, ETraceTypeQuery::TraceTypeQuery6,
	true, {this, Owner}, EDrawDebugTrace::None, TraceResults, true);
	FHitResult CenteredHitResult;
	FVector EyesLocation;
	FRotator EyesRotation;
	GetActorEyesViewPoint(EyesLocation, EyesRotation);
	UKismetSystemLibrary::LineTraceSingle(GetWorld(), EyesLocation,
		EyesLocation + EyesRotation.Vector() * AutotargetingRange,
		ETraceTypeQuery::TraceTypeQuery6, true, {this, Owner}, EDrawDebugTrace::None,
		CenteredHitResult, true);
	AActor* CenteredActor = nullptr;
	if(CenteredHitResult.bBlockingHit)
	{
		CenteredActor = CenteredHitResult.GetActor();
	}
	
	TTuple<float, UTargetInformationComponent*> BestResult;
	BestResult.Key = 0.f;
	BestResult.Value = nullptr;
	for(FHitResult TraceResult : TraceResults)
	{
		if(!TraceResult.bBlockingHit) continue; //all relevant meshes are set to block destructible objects...
		UActorComponent* Component = TraceResult.GetActor()->GetComponentByClass(UTargetInformationComponent::StaticClass());
		if(!IsValid(Component)) continue; //... and have a target information component
		UTargetInformationComponent* TargetInfoComp = CastChecked<UTargetInformationComponent>(Component);

		//Get the actors center
		FVector ActorCenter;
		FVector Extent;
		TraceResult.GetActor()->GetActorBounds(true, ActorCenter, Extent);

		//whether the target is on screen
		float OffsetFromForward = FVector::DotProduct(EyesRotation.Vector(),
			UKismetMathLibrary::GetDirectionUnitVector(EyesLocation, ActorCenter));
		if(UKismetMathLibrary::DegAcos(OffsetFromForward) > GetFieldOfView()/2.f) continue;
		
		//whether the TargetInfoComp is not occluded
		FHitResult VisibilityTrace;
		UKismetSystemLibrary::LineTraceSingle(GetWorld(), EyesLocation, ActorCenter,
		ETraceTypeQuery::TraceTypeQuery1, true, {this, Owner}, EDrawDebugTrace::None,
		VisibilityTrace, true);
		if(VisibilityTrace.bBlockingHit && VisibilityTrace.GetActor() != TraceResult.GetActor())
		{
			//If the first probe didn't return visible, it doesn't mean the actor is really occluded
			TArray Locations({ActorCenter + Extent, ActorCenter - Extent,
				ActorCenter + FVector(Extent.X, Extent.Y, -Extent.Z), ActorCenter + FVector(Extent.X, -Extent.Y, Extent.Z),
				ActorCenter + FVector(-Extent.X, Extent.Y, Extent.Z), ActorCenter - FVector(Extent.X, Extent.Y, -Extent.Z),
				ActorCenter - FVector(Extent.X, -Extent.Y, Extent.Z), ActorCenter - FVector(-Extent.X, Extent.Y, Extent.Z)});
			if(!AreMultipleVisible(TraceResult.GetActor(), EyesLocation, Locations, 2)) continue;
		}
		
		
		//Generate a score for the target priority
		float TotalScore = 0.f;
		TotalScore += 0.75f * OffsetFromForward; //together with centered actor we can still reach 1.f
		if(CenteredActor == TraceResult.GetActor()) TotalScore += 0.25f;
		if(GetWorld()->RealTimeSeconds - InputDirection.Key <= RememberInputDirectionTime)
			TotalScore += 3.f * FVector::DotProduct(InputDirection.Value,
				UKismetMathLibrary::GetDirectionUnitVector(GetActorLocation(), ActorCenter));
		TotalScore += 1.f - FVector::Distance(GetActorLocation(), ActorCenter)/AutotargetingRange;
		if(TargetInfoComp->GetTargetState()) TotalScore += 0.5f;
		TotalScore *= TargetInfoComp->GetTargetPriority();

		//only keep the best score
		if(TotalScore > BestResult.Key)
		{
			BestResult.Key = TotalScore;
			BestResult.Value = TargetInfoComp;
		}
	}

	//tell the target components who the new target is
	if(CurrentTarget != BestResult.Value)
	{
		if(IsValid(CurrentTarget)) CurrentTarget->SetTargetState(false, FSetTargetStateKey());
		if(IsValid(BestResult.Value))
		{
			CurrentTarget = BestResult.Value;
			CurrentTarget->SetTargetState(true, FSetTargetStateKey());
		}
		else CurrentTarget = nullptr;
	}

#if WITH_EDITORONLY_DATA
	//Draw debugging information
	if(bIsDebugging)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), AutotargetingRange, 100, FColor(0, 255, 0));
		if(GetWorld()->RealTimeSeconds - InputDirection.Key <= RememberInputDirectionTime) DrawDebugDirectionalArrow(GetWorld(),
			GetActorLocation(),GetActorLocation() + InputDirection.Value * 100.f,
			50.f, FColor(0, 0, 255), false, -1.f, 0, 5.f);
		if(IsValid(CurrentTarget)) DrawDebugSphere(GetWorld(), CurrentTarget->GetComponentLocation(), 50.f,
			20, FColor(100, 255, 100));
	}
#endif
}

void APlayerCharacter::OnSelectMotionWarpingTarget(const FAttackProperties& Properties)
{
	if(IsValid(CurrentTarget))
	{
		FVector Direction = CurrentTarget->GetComponentLocation() - GetActorLocation();
		if(Direction.Length() <= Properties.MaximalMotionWarpingDistance)
			SuckToTargetComponent->SetWarpTargetFaceTowards(CurrentTarget);
		
		else
		{
			Direction.Normalize();
			//MotionWarpingComponent->RemoveWarpTarget("AttackComponent");
			SuckToTargetComponent->SetWarpTargetFaceTowards(
				GetActorLocation() + Direction * Properties.MaximalMotionWarpingDistance, GetActorLocation());
		}
	}
	else
	{
		FVector AttackPosition;
		if(GetWorld()->RealTimeSeconds - InputDirection.Key > RememberInputDirectionTime){
			FVector PlayerLocation;
			FRotator PlayerRotation;
			GetActorEyesViewPoint(PlayerLocation, PlayerRotation);
			FVector PlayerForward = PlayerRotation.Vector();
			PlayerForward.Z = 0.f;
			if(PlayerForward.Length() <= 0.01f)
			{
				Super::GetActorEyesViewPoint(PlayerLocation, PlayerRotation);
				PlayerForward = PlayerRotation.Vector();
			}
			else PlayerForward.Normalize();
			AttackPosition = GetActorLocation() + PlayerForward * Properties.MaximalMotionWarpingDistance/2.f;
		} 
		else AttackPosition = GetActorLocation() + InputDirection.Value * Properties.MaximalMotionWarpingDistance/2.f;
		SuckToTargetComponent->SetWarpTargetFaceTowards(AttackPosition, GetActorLocation());
	}
}
