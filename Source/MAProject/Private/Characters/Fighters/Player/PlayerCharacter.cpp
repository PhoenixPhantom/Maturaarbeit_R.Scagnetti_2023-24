// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Player/PlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

APlayerCharacter::APlayerCharacter() : bIsRunning(false)
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

void APlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void APlayerCharacter::GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	OutLocation = FollowCamera->GetComponentLocation();
	OutRotation = FollowCamera->GetComponentRotation();
}

void APlayerCharacter::PreSpawnSetup(FCharacterStats* PropertiesSource, FPreSpawnSetupKey Key)
{
	CharacterStats = PropertiesSource;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	FollowCamera->SetFieldOfView(FieldOfView);
}

void APlayerCharacter::Destroyed()
{
	Super::Destroyed();
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
}

void APlayerCharacter::HeavyAttack(const FInputActionValue& Value)
{
}

void APlayerCharacter::SkillAttack(const FInputActionValue& Value)
{
}

void APlayerCharacter::UltimateAttack(const FInputActionValue& Value)
{
}

void APlayerCharacter::SpeedUpDash(const FInputActionValue& Value)
{
	if(!CurrentlyAvailableInputs.bCanSprint) return;
	//we only have (and should only) to set variables one time
	if(!bIsRunning)
	{
		GetCharacterMovement()->MaxWalkSpeed = CharacterStats->RunSpeed.GetResulting();
	
		//Since on ongoing: dash will be called, we can assume that the player moves every time you dash, so
		//a dash will always allows you to walk after execution because you've already been displaced)
		CurrentlyAvailableInputs.MovementProperties.bCanWalk = true;
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
	GetCharacterMovement()->MaxWalkSpeed = CharacterStats->WalkSpeed.GetResulting();
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (IsValid(Controller) && CurrentlyAvailableInputs.MovementProperties.bCanWalk)
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

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (IsValid(Controller) && CurrentlyAvailableInputs.bFreeCameraAdjustment)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APlayerCharacter::CameraZoom(const FInputActionValue& Value)
{
	const float Movement = Value.Get<float>();
	if(CurrentlyAvailableInputs.bFreeCameraAdjustment)
	{
		unimplemented();
		CameraBoom->TargetArmLength += Movement; //CameraZoomSpeed;
	}
}

void APlayerCharacter::Aim(const FInputActionValue& Value)
{
	if(CurrentlyAvailableInputs.bFreeCameraAdjustment) unimplemented();
}

void APlayerCharacter::OpenPauseMenu(const FInputActionValue& Value)
{
	unimplemented();
}
