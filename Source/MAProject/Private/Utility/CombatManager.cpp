// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/CombatManager.h"

#include "Characters/Fighters/Attacks/AttackTree/AttackNode.h"
#include "Characters/Fighters/Opponents/OpponentCharacter.h"
#include "Characters/Fighters/Opponents/AI/OpponentController.h"
#include "Characters/Fighters/Player/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/Sound/GlobalSoundManager.h"


bool FAggressorInfo::operator==(const FAggressorInfo& AggressionData) const
{
	return RequestedAttack == AggressionData.RequestedAttack && Aggressor == AggressionData.Aggressor &&
		RequestedTokens == AggressionData.RequestedTokens;
}

FScoredAggressorInfo::FScoredAggressorInfo(AOpponentCharacter* NewHolder, UAttackNode* AttackTreeNode,
	float NewScore, uint32 NewTokens) : FAggressorInfo(NewHolder, AttackTreeNode, NewTokens), Score(NewScore)
{
}

// Sets default values
ACombatManager::ACombatManager() : PlayerCharacter(nullptr), SoundManager(nullptr), PendingOutOfCombatDuration(10.f),
                                   MaxAggressionTokens(2),
                                   AvailableAggressionTokens(MaxAggressionTokens)
{
#if WITH_EDITORONLY_DATA
	PrimaryActorTick.bCanEverTick = true;
#else
	PrimaryActorTick.bCanEverTick = false;
#endif
}

ECombatParticipantStatus ACombatManager::GetParticipationStatus(AFighterCharacter* Character) const
{
	if(Character == PlayerCharacter) return ECombatParticipantStatus::Player;
	if(PassiveParticipants.Contains(Character)) return ECombatParticipantStatus::Passive;
	if(ActiveParticipants.Contains(Character)) return ECombatParticipantStatus::Active;
	return ECombatParticipantStatus::NotRegistered;
}

void ACombatManager::RegisterCombatParticipant(APlayerCharacter* PlayerParticipant, FManageCombatParticipantsKey Key)
{
	//when the player character is unregistered, combat for all NPCs attacking them is ended
	if(PlayerParticipant == nullptr)
	{
		TArray<AOpponentCharacter*> InCombat = ActiveParticipants;
		InCombat.Append(PassiveParticipants);
		for(AOpponentCharacter* CurrentlyInCombat : InCombat)
		{
			if(CurrentlyInCombat->GetCombatTarget() != PlayerCharacter) continue;
			CastChecked<AOpponentController>(CurrentlyInCombat->GetController())->ForceEndCombat(true, FForceEndCombatKey());
		}
	}
	
	PlayerCharacter = PlayerParticipant;
}

bool ACombatManager::RegisterCombatParticipant(AOpponentCharacter* Participant, FManageCombatParticipantsKey Key)
{
	if(ECombatParticipantStatus::NotRegistered != GetParticipationStatus(Participant)) return false;
	if(FTimerHandle* TimerHandle = PendingOutOfCombat.Find(Participant); TimerHandle == nullptr)
	{
		if(IsValid(SoundManager) && PassiveParticipants.IsEmpty() && ActiveParticipants.IsEmpty())
		{
			SoundManager->EnterCombatState(FSetCombatStateKey());
			PlayerCharacter->SetIsRestoringHealth(false, FSetIsRestoringHealthKey());
		}
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(*TimerHandle);
		PendingOutOfCombat.Remove(Participant);
	}
	PassiveParticipants.Add(Participant);
	//try to grant the tokens
	GrantTokens(FAggressorInfo(Participant, Participant->GetRandomValidAttack(), Participant->GetRequestedTokens()));
	return true;
}

void ACombatManager::UnregisterCombatParticipant(AOpponentCharacter* Participant, bool SetToPending, FManageCombatParticipantsKey Key)
{
	if(AnticipatedActive.Aggressor == Participant) AnticipatedActive = FAggressorInfo();
	//we can't use ReleaseAggressionTokens as this could lead to token redistribution and Participant not becoming passive
	const bool WasActiveParticipant = RemoveAggressionTokens(Participant);
	PassiveParticipants.RemoveSwap(Participant);
	if(WasActiveParticipant) AttemptDistributeFreeTokens();
	if(!SetToPending)
	{
		FullyExitFromCombat(Participant);
		return;
	}
	FTimerHandle& CorrespondingHandle = PendingOutOfCombat.Add(Participant);
	GetWorld()->GetTimerManager().SetTimer(CorrespondingHandle, [Local = this, Participant]
	{
		if(!IsValid(Local)) return;
		Local->OnOutOfCombat(Participant);
	}
	, PendingOutOfCombatDuration, false);
}

void ACombatManager::ReleaseAggressionTokens(AOpponentCharacter* Participant, FManageAggressionTokensKey Key)
{
	if(RemoveAggressionTokens(Participant))	AttemptDistributeFreeTokens();
}

#if WITH_EDITORONLY_DATA
void ACombatManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if(bIsDebugging)
	{
		for(const AOpponentCharacter* OpponentCharacter : PassiveParticipants)
		{
			UKismetSystemLibrary::DrawDebugCircle(GetWorld(), OpponentCharacter->GetActorLocation(), 100.f,
		50, FLinearColor(0, 0, 255), 0, 20.f, FVector(0, 1, 0),
		FVector(1, 0, 0));
		}
		for(const AOpponentCharacter* OpponentCharacter : ActiveParticipants)
		{
			UKismetSystemLibrary::DrawDebugCircle(GetWorld(), OpponentCharacter->GetActorLocation(), 100.f,
		50, FLinearColor(255, 0, 0), 0, 20.f, FVector(0, 1, 0),
		FVector(1, 0, 0));
		}
	}
}
#endif

// Called when the game starts or when spawned
void ACombatManager::BeginPlay()
{
	Super::BeginPlay();

	AvailableAggressionTokens = MaxAggressionTokens;

	//There can only be one combat manager at any given time to prevent logic problems
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACombatManager::StaticClass(), Actors);
	check(Actors.Num() == 1 && Actors[0] == this);


	//get the sound manager
	Actors.Empty();
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGlobalSoundManager::StaticClass(), Actors);
	SoundManager = Cast<AGlobalSoundManager>(Actors[0]);
}

bool ACombatManager::RemoveAggressionTokens(AOpponentCharacter* Participant)
{
	if(!ActiveParticipants.Contains(Participant)) return false;
	AvailableAggressionTokens += Participant->GetRequestedTokens();
	if(bIsDebugging)
	{
		UKismetSystemLibrary::DrawDebugCircle(GetWorld(), Participant->GetActorLocation() +
			FVector(0.f, 0.f, 50.f), 100.f,
		50, FLinearColor(0, 255, 0), 1.f, 10.f, FVector(0, 1, 0),
		FVector(1, 0, 0));
	}
	return MakePassiveParticipant(ActiveParticipants.Find(Participant));
}

bool ACombatManager::GrantTokens(const FAggressorInfo& AggressorInfo)
{
	if(!PassiveParticipants.Contains(AggressorInfo.Aggressor) || ActiveParticipants.Contains(AggressorInfo.Aggressor))
	{
		//non-passive (active) participants cannot be granted a token 
		checkNoEntry();
		return false;
	}
	if(AggressorInfo.RequestedTokens > AvailableAggressionTokens) return false;
	AvailableAggressionTokens -= AggressorInfo.RequestedTokens;
	MakeActiveParticipant(PassiveParticipants.Find(AggressorInfo.Aggressor));
	AggressorInfo.Aggressor->SetRequestedAttack(AggressorInfo.RequestedAttack, FRequestAttackKey());
	AggressorInfo.Aggressor->ExecuteOnAggressionTokensGranted(FExecuteOnAggressionTokensGrantedKey());
	return true;
}

bool ACombatManager::MakeActiveParticipant(int32 Index)
{
	if(!PassiveParticipants.IsValidIndex(Index))
	{
		checkNoEntry();
		return false;
	}
	ActiveParticipants.Add(PassiveParticipants[Index]);
	PassiveParticipants.RemoveAtSwap(Index);
	return true;
}

bool ACombatManager::MakePassiveParticipant(int32 Index)
{
	if(!ActiveParticipants.IsValidIndex(Index))
	{
		checkNoEntry();
		return false;
	}
	PassiveParticipants.Add(ActiveParticipants[Index]);
	ActiveParticipants.RemoveAtSwap(Index);
	return true;
}

float ACombatManager::GetAttackValue(UAttackNode* AttackNode, AOpponentCharacter* Attacker)
{
	if(!IsValid(AttackNode) || !IsValid(Attacker)) return -1.f;
	//getOverallValue will often be around 2 and the character's attack stat is around 100
	//Since we want the attack score to maximally be 6, we multiply with  ~6/200
	return 0.03f * static_cast<float>(Attacker->GetCharacterStats()->Attack.GetResulting()) *
		AttackNode->GetAttackProperties().GetOverallValue();
	//we just look at the "default attack", even if this might not be accurate when using contextual attacks
}

void ACombatManager::AttemptDistributeFreeTokens()
{
	if(IsValid(AnticipatedActive.Aggressor) && IsValid(AnticipatedActive.RequestedAttack))
	{
		if(!GrantTokens(AnticipatedActive)) return;
	}
	
	//Generate a list of participants trying to become aggressive sorted from best to worst
	TList<FScoredAggressorInfo>* BestOption = nullptr;
	for(AOpponentCharacter* Participant : PassiveParticipants)
	{
		float Score = Participant->GenerateAggressionScore(PlayerCharacter);
		//A score < 0.f means that the given participant cannot become aggressive, so it is not relevant
		if(Score >= 0.f && Participant->GetRequestedTokens() <= MaxAggressionTokens)
		{
			UAttackNode* RequestedAttack = Participant->GetRandomValidAttack();
			Score += GetAttackValue(RequestedAttack, Participant);

			//Add the information generated to the list in the correct place
			const FScoredAggressorInfo ScoredAggressorInfo(Participant, RequestedAttack, Score, Participant->GetRequestedTokens());
			TList<FScoredAggressorInfo>** CurrentElement = &BestOption;
			while(true)
			{
				if(*CurrentElement == nullptr)
				{
					*CurrentElement = new TList(ScoredAggressorInfo);
					break;
				}
				if(Score > (*CurrentElement)->Element.Score)
				{
					TList<FScoredAggressorInfo>* NextElement = *CurrentElement;
					*CurrentElement = new TList(ScoredAggressorInfo, NextElement);
					break;
				}
				CurrentElement = &(*CurrentElement)->Next;
			}
		}
	}

	TList<FScoredAggressorInfo>* CurrentOption = BestOption;
	while(true)
	{
		if(CurrentOption == nullptr) break;
		//Try grant tokens to the entity, thereby checking whether that is even possible
		if(!GrantTokens(CurrentOption->Element))
		{
			if(CurrentOption->Next != nullptr && CurrentOption->Next->Element.RequestedTokens <= AvailableAggressionTokens)
			{
				//if an entity requires more tokens than what is left over, and there are still other entities that might
				//be satisfied with the available amount of tokens, we queue the entity (to be guaranteed to be the
				//next one to receive a token) before stopping token distribution.
				//This prevents entities requiring a high amount of tokens from never being able to get active.
				AnticipatedActive = static_cast<FAggressorInfo>(CurrentOption->Next->Element);
			}
			else AnticipatedActive = FAggressorInfo();
			break;
		}
		CurrentOption = CurrentOption->Next;
	}	
}

void ACombatManager::FullyExitFromCombat(AOpponentCharacter* OpponentCharacter)
{
	OpponentCharacter->ResetAllStats(FResetOpponentStatsKey());
	//if the pending out of combat participant is the last character to exit from combat, we change the sound state to non-combat
	if(PassiveParticipants.IsEmpty() && ActiveParticipants.IsEmpty())
	{
		if(IsValid(SoundManager))  SoundManager->EndCombatState(FSetCombatStateKey());
		PlayerCharacter->SetIsRestoringHealth(true, FSetIsRestoringHealthKey());
	}
}

void ACombatManager::OnOutOfCombat(AOpponentCharacter* Participant)
{
	if(!IsValid(Participant))
	{
		PendingOutOfCombat.Remove(Participant);
	}
	if(PendingOutOfCombat.Contains(Participant))
	{
		FullyExitFromCombat(Participant);
		PendingOutOfCombat.FindAndRemoveChecked(Participant);
	}
	
}

