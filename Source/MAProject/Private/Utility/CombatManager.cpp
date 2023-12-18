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
	TArray<AOpponentCharacter*> InCombat = ActiveParticipants;
	InCombat.Append(PassiveParticipants);
	for(AOpponentCharacter* CurrentlyInCombat : InCombat)
	{
		if(CurrentlyInCombat->GetCombatTarget() != PlayerParticipant) continue;
		CastChecked<AOpponentController>(CurrentlyInCombat->GetController())->ForceEndCombat(true, FForceEndCombatKey());
	}
	PlayerCharacter = PlayerParticipant;
}

bool ACombatManager::RegisterCombatParticipant(AOpponentCharacter* Participant, FManageCombatParticipantsKey Key)
{
	if(ECombatParticipantStatus::NotRegistered != GetParticipationStatus(Participant)) return false;
	if(FTimerHandle* TimerHandle = PendingOutOfCombat.Find(Participant); TimerHandle == nullptr)
	{
		if(IsValid(SoundManager) && PassiveParticipants.IsEmpty() && ActiveParticipants.IsEmpty())
			SoundManager->EnterCombatState(FSetCombatStateKey());
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(*TimerHandle);
		PendingOutOfCombat.Remove(Participant);
	}
	PassiveParticipants.Add(Participant);
	RequestToken(Participant);
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
		if(IsValid(SoundManager) && PassiveParticipants.IsEmpty() && ActiveParticipants.IsEmpty())
			SoundManager->EndCombatState(FSetCombatStateKey());
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
	//getOverallValue is normally under 12.5 and the character's attack stat is around 100
	//Since we want the attack score to weigh about half as much as the aggression score, we multiply with  ~6/12500
	//(6 for the expected highest amount for the optimal target)
	return 0.0004f * static_cast<float>(Attacker->GetCharacterStats()->Attack.GetResulting()) *
		AttackNode->GetAttackProperties().GetOverallValue(); //we just look at the "default attack", even if this might not always make sense
}

void ACombatManager::AttemptDistributeFreeTokens()
{
	if(IsValid(AnticipatedActive.Aggressor) && IsValid(AnticipatedActive.RequestedAttack))
	{
		if(!GrantTokens(AnticipatedActive)) return;
	}
	
	//The resulting sorted list of options from best to worst
	TList<FScoredAggressorInfo>* BestOption = nullptr;
	for(AOpponentCharacter* Participant : PassiveParticipants)
	{
		float Score = Participant->GenerateAggressionScore(PlayerCharacter);
		//A score < 0.f means that the given participant cannot become aggressive, so it is not relevant
		if(Score >= 0.f)
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
			if(AvailableAggressionTokens >= 1 && CurrentOption->Next != nullptr)
			{
				//if an entity requires more tokens than what is left over, and there are still other entities that might
				//be satisfied with the available amount of tokens, we queue the entity (to be guaranteed to be the
				//next one to receive a token) before stopping token distribution
				//(this should prevent entities requiring a high amount of tokens from never being able to get active,
				//but obviously brings with it some other problems)
				AnticipatedActive = static_cast<FAggressorInfo>(CurrentOption->Next->Element);
			}
			else AnticipatedActive = FAggressorInfo();
			break;
		}
		CurrentOption = CurrentOption->Next;
	}	
}

void ACombatManager::RequestToken(AOpponentCharacter* Requestor)
{
	uint32 OverridableTokens = MaxAggressionTokens;
	
	float RequestorScore = Requestor->GenerateAggressionScore(PlayerCharacter);
	if(RequestorScore < 0.f) return;
	
	UAttackNode* DesiredAttack = Requestor->GetRandomValidAttack();
	RequestorScore += GetAttackValue(DesiredAttack, Requestor);
	
	for(AOpponentCharacter* Participant : ActiveParticipants)
	{
		float Score = Participant->GenerateAggressionScore(PlayerCharacter);
		//A score < 0.f means that the given participant cannot become aggressive, so it is not relevant
		if(Score >= 0.f)
		{
			UAttackNode* RequestedAttack = Participant->GetRequestedAttack();
			Score += GetAttackValue(RequestedAttack, Participant);
			//only lower priorities can be overridden by the new requestor
			if(Score >= RequestorScore) OverridableTokens -= Participant->GetRequestedTokens();
		}
	}

	if(OverridableTokens < Requestor->GetRequestedTokens()) return;
	
	GrantTokens(FAggressorInfo(Requestor, DesiredAttack, Requestor->GetRequestedTokens()));
}

void ACombatManager::OnOutOfCombat(AOpponentCharacter* Participant)
{
	if(!IsValid(Participant))
	{
		PendingOutOfCombat.Remove(Participant);
	}
	//if the pending out of combat participant is the last character to exit from combat, we change the sound state to non-combat
	if(IsValid(SoundManager) && PassiveParticipants.IsEmpty() && ActiveParticipants.IsEmpty()
		&& PendingOutOfCombat.Contains(Participant))
	{
		PendingOutOfCombat.FindAndRemoveChecked(Participant);
		SoundManager->EndCombatState(FSetCombatStateKey());
	}
}

