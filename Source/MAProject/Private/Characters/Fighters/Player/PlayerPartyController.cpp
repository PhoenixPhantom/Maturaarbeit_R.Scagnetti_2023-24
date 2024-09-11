// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Fighters/Player/PlayerPartyController.h"

#include "Utility/CombatManager.h"
#include "Characters/Fighters/Player/PlayerCharacter.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

APlayerPartyController::APlayerPartyController() : PawnStartTransform(FVector(NAN)), CurrentYawRotSpeed(0.0),
	CurrentPitchRotSpeed(0.0), ActorRequestedInView(nullptr), CombatManager(nullptr), MaxYawAccel(100.0),
	MaxPitchAccel(75.0)
{
	OnPlayerCameraMovedPreconfigured.BindUObject(this, &APlayerPartyController::OnPlayerCameraMoved);
}

void APlayerPartyController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	//only do automated camera management, if the player are not controlling the camera themselves
	if(GetWorld()->RealTimeSeconds - CameraInterventionData.Timestamp > 1.0)
	{
		AutoControlCameraRotation(DeltaSeconds);
	}
	else SetCurrentRotSpeed(0.0, 0.0);
	
}

void APlayerPartyController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	APlayerCharacter* TargetCharacter = Cast<APlayerCharacter>(InPawn);
	if(!IsValid(TargetCharacter)) return;
	
	CombatManager->RegisterCombatParticipant(TargetCharacter, FManageCombatParticipantsKey());
	CurrentCharacter = TargetCharacter;
	CurrentCharacter->EnableInput(this); //Input seems to be disabled by default

}

void APlayerPartyController::OnUnPossess()
{
	APawn* LocalOldPawn = GetPawn();
	if(IsValid(LocalOldPawn))
	{
		//if the old pawn was a player character, we have to unregister them from combat before destroying them
		if(IsValid(CurrentCharacter))
		{
			CombatManager->RegisterCombatParticipant(static_cast<APlayerCharacter*>(nullptr), FManageCombatParticipantsKey());
		}
		
		//remove redundant player characters
		LocalOldPawn->Destroy();
	}
	Super::OnUnPossess();
	CurrentCharacter = nullptr;
}

void APlayerPartyController::BeginPlay()
{

	Super::BeginPlay();

	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACombatManager::StaticClass(), Actors);
	check(Actors.Num() == 1);
	CombatManager = CastChecked<ACombatManager>(Actors[0]);

	const APlayerCharacter* TargetCharacter = CastChecked<APlayerCharacter>(PartyMemberClass.GetDefaultObject());
	PartyMemberStats.FromBase(TargetCharacter->GetCharacterBaseStats(), PartyMemberModifiers, this);

	FTransform TargetTransform;
	//If we have already a saved location
	if(PawnStartTransform.IsValid())
	{
		TargetTransform = PawnStartTransform;
	}
	//if this is the first character that is spawned (ever), it should be spawned at the PlayerStart
	//(which can optionally be marked with the tag "ActiveSpawn")
	else
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), FoundActors);
		check(!FoundActors.IsEmpty());
		const APlayerStart* ChosenPlayerStart = nullptr;
		for(AActor* Actor : FoundActors)
		{
			ChosenPlayerStart = CastChecked<APlayerStart>(Actor);
			if(ChosenPlayerStart->PlayerStartTag == "ActiveSpawn") break;
		}
		TargetTransform = ChosenPlayerStart->GetTransform();
		UE_LOG(LogTemp, Log, TEXT("Spawn transform is: %s"), *TargetTransform.ToString());
	}

	APlayerCharacter* NewCharacter = GetWorld()->SpawnActorDeferred<APlayerCharacter>(PartyMemberClass.Get(),
		TargetTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	
	NewCharacter->PreSpawnSetup(&PartyMemberStats, &PlayerUserSettings, OnPlayerCameraMovedPreconfigured,
		GetGenericTeamId(), FPreSpawnSetupKey());
#if WITH_EDITORONLY_DATA
	NewCharacter->SetIsDebugging(bIsDebugging);
#endif
	NewCharacter->FinishSpawning(TargetTransform);
	
	Possess(NewCharacter);
}

void APlayerPartyController::SetCurrentRotSpeed(double Yaw, double Pitch)
{
	CurrentYawRotSpeed = Yaw;
	CurrentPitchRotSpeed = Pitch;
}

void APlayerPartyController::AutoControlCameraRotation(float DeltaSeconds)
{
	FRotator TargetRotator(NAN);
	if(IsValid(CurrentCharacter))
	{
		LockOnTarget(TargetRotator);
		OrientTowardsMovement(TargetRotator);
	}

	if(TargetRotator.ContainsNaN())
	{
		SetCurrentRotSpeed(0.0, 0.0);
		return;
	}
	const FRotator OriginalRotator = GetControlRotation();
	double DesiredDeltaYaw = TargetRotator.Yaw - OriginalRotator.Yaw;
	double DesiredDeltaPitch = TargetRotator.Pitch - OriginalRotator.Pitch;
	//use shortest path
	if(DesiredDeltaYaw >= 180.0) DesiredDeltaYaw = 360.0 - DesiredDeltaYaw;
	if(DesiredDeltaYaw <= -180.0) DesiredDeltaYaw = 360.0 + DesiredDeltaYaw;
	if(DesiredDeltaPitch >= 180.0) DesiredDeltaPitch = 360.0 - DesiredDeltaPitch;
	if(DesiredDeltaPitch <= -180.0) DesiredDeltaPitch = 360.0 + DesiredDeltaPitch;

	//pitch must always stay between -90 and 90 (since the controller doesn't allow for more
	if(DesiredDeltaPitch >= 90.0) DesiredDeltaPitch = 180.0 - DesiredDeltaPitch;
	if(DesiredDeltaPitch <= -90.0) DesiredDeltaPitch = -180.0 - DesiredDeltaPitch;

	//asymptotic interpolation behavior
	DesiredDeltaYaw *= FMath::Min(DeltaSeconds * 1.5, 1.0); 
	DesiredDeltaPitch *= DeltaSeconds;

	//Cap max acceleration to prevent stuttery 1 to 100 accelerations
	const double AllowedDeltaYaw = FMath::Min(DesiredDeltaYaw,
		(MaxYawAccel * DeltaSeconds + CurrentYawRotSpeed) * DeltaSeconds);
	const double AllowedDeltaPitch = FMath::Min(DesiredDeltaPitch,
		(MaxPitchAccel * DeltaSeconds + CurrentPitchRotSpeed) * DeltaSeconds);

	SetControlRotation({OriginalRotator.Pitch + AllowedDeltaPitch, OriginalRotator.Yaw +  AllowedDeltaYaw,
		OriginalRotator.Roll});
	SetCurrentRotSpeed(AllowedDeltaYaw / DeltaSeconds, AllowedDeltaPitch / DeltaSeconds);
}

void APlayerPartyController::LockOnTarget(FRotator& DesiredRotation)
{
	//if we have a valid target, we lock on to it by default
	if(ActorRequestedInView != CurrentCharacter->GetCurrentTarget() && IsValid(CurrentCharacter->GetCurrentTarget()))
	{
		//to reduce noise on target change we register a target change the same way as we would register camera movement
		//by the player
		CameraInterventionData.Timestamp = GetWorld()->RealTimeSeconds;
	
		if(!TrySetRequestedInView(CurrentCharacter->GetCurrentTarget())) return;
	}
	if(!IsValid(ActorRequestedInView)) return;

	const FVector& PlayerOpponentVector = ActorRequestedInView->GetActorLocation() - CurrentCharacter->GetActorLocation();
	const FVector& DesiredCameraDirection = PlayerOpponentVector.Cross(FVector(0.f, 0.f, 1.f));
		

	FVector CinematicViewVector; 
	//choose the direction that requires less than a 180° turn
	if(FVector::DotProduct(DesiredCameraDirection, GetControlRotation().Vector()) >= 0.f)
		CinematicViewVector = DesiredCameraDirection;
	else CinematicViewVector = -DesiredCameraDirection;

	const float RelevantDistance = PlayerOpponentVector.Length() - CurrentCharacter->GetSimpleCollisionRadius() -
		ActorRequestedInView->GetSimpleCollisionRadius();
	//balance the camera position (to the side) with actually seeing the view target
	DesiredRotation = UKismetMathLibrary::VLerp(PlayerOpponentVector, CinematicViewVector, 
			std::max(0.0, 1.0 - RelevantDistance/750.0)).Rotation();
}

void APlayerPartyController::OrientTowardsMovement(FRotator& DesiredRotation) const
{
	//this cannot override or change any other inputs
	//and only takes effect when "smoothly turning" and not immediate jumps, preventing forward orientation when not wanted
	if(DesiredRotation.ContainsNaN())
	{
		const FVector& PendingMovementInput = CurrentCharacter->GetMovementComponent()->GetPendingInputVector();
		//squared length is slightly faster and we compare it to the constant speed 10cm/s anyways
		//(so now we compare with 100cm/s)
		if(PendingMovementInput.SquaredLength() >= 100.0 &&
			PendingMovementInput.Rotation().Equals(GetControlRotation(), 89.0))
				DesiredRotation = CurrentCharacter->GetVelocity().Rotation();
	}
}

bool APlayerPartyController::TrySetRequestedInView(AActor* RequestedInView)
{
	if(IsValid(RequestedInView))
	{
		FVector CameraLocation;
		FRotator CameraRotation;
		CurrentCharacter->GetActorEyesViewPoint(CameraLocation, CameraRotation);

		//only characters indicates should be focused on by looking at them will be followed
		if (UKismetMathLibrary::DegAcos(FVector::DotProduct(CameraRotation.Vector(),
			UKismetMathLibrary::GetDirectionUnitVector(CameraLocation,RequestedInView->GetActorLocation())))
			<= CurrentCharacter->GetFieldOfView() / 2.f)
		{
			ActorRequestedInView = RequestedInView;
			return true;
		}
	}
	ActorRequestedInView = nullptr;
	return false;
}

void APlayerPartyController::OnPlayerCameraMoved(const FVector2D& LookInputAxis)
{
	//Determine what the most likely intention of the player was 
	TrySetRequestedInView(CurrentCharacter->GetCurrentTarget());
	CameraInterventionData.Timestamp = GetWorld()->RealTimeSeconds;
}

void APlayerPartyController::Respawn()
{
	PartyMemberStats.Reset();

	FTransform TargetTransform;
	//reload at the last saved location if possible
	if(PawnStartTransform.IsValid())
	{
		TargetTransform = PawnStartTransform;
	}
	//if nothing has been saved jet, the character should be spawned at the PlayerStart
	//(which can optionally be marked with the tag "ActiveSpawn")
	else
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), FoundActors);
		check(!FoundActors.IsEmpty());
		const APlayerStart* ChosenPlayerStart = nullptr;
		for(AActor* Actor : FoundActors)
		{
			ChosenPlayerStart = CastChecked<APlayerStart>(Actor);
			if(ChosenPlayerStart->PlayerStartTag == "ActiveSpawn") break;
		}
		TargetTransform = ChosenPlayerStart->GetTransform();
		UE_LOG(LogTemp, Log, TEXT("Spawn transform is: %s"), *TargetTransform.ToString());
	}

	APlayerCharacter* NewCharacter = GetWorld()->SpawnActorDeferred<APlayerCharacter>(PartyMemberClass.Get(),
		TargetTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	
	NewCharacter->PreSpawnSetup(&PartyMemberStats, &PlayerUserSettings, OnPlayerCameraMovedPreconfigured,
		GetGenericTeamId(), FPreSpawnSetupKey());
#if WITH_EDITORONLY_DATA
	NewCharacter->SetIsDebugging(bIsDebugging);
#endif
	NewCharacter->FinishSpawning(TargetTransform);

	
	Possess(NewCharacter);
}

#if WITH_EDITORONLY_DATA
void APlayerPartyController::ToggleDebugging()
{
	bIsDebugging = !bIsDebugging;
	if(APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetPawn()); IsValid(PlayerCharacter)){
		PlayerCharacter->SetIsDebugging(bIsDebugging);
	}
}
#endif
