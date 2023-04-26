// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/Savegame/SavableObjectIDGenerator.h"

#include "Kismet/GameplayStatics.h"
#include "Utility/Savegame/SavableObjectMarkerComponent.h"

DEFINE_LOG_CATEGORY(LogUniqueWorldId);

uint64 GetLowestValidID(const TArray<uint64>& IDsInUse)
{
	uint64 LowestId = std::numeric_limits<uint64>::max();
	if(IDsInUse.IsEmpty()) return LowestId;
	for(const uint64 Id : IDsInUse)
	{
		if(Id < LowestId) LowestId = Id;
	}

	return LowestId;
}

// Sets default values
ASavableObjectIDGenerator::ASavableObjectIDGenerator() : bAllowReusingIndices(true)
{
	PrimaryActorTick.bCanEverTick = false;
}

bool ASavableObjectIDGenerator::IsEditorOnly() const
{
	return true;
}

// Called when the game starts or when spawned
void ASavableObjectIDGenerator::BeginPlay()
{
	Super::BeginPlay();
	Recalculate();	
}

void ASavableObjectIDGenerator::Recalculate() const
{
	//Collect the new actors to add
	TArray<uint64> IDsInUse;
	TArray<USavableObjectMarkerComponent*> NewObjects;
	TArray<AActor*> WorldActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), WorldActors);
	for(const AActor* Actor : WorldActors)
	{
		//Only interested in Savable actors
		if(!IsValid(Actor)) continue;
		UActorComponent* ObjectComponent = Actor->GetComponentByClass(USavableObjectMarkerComponent::StaticClass());
		if(!IsValid(ObjectComponent)) continue;
		USavableObjectMarkerComponent* CallRef = CastChecked<USavableObjectMarkerComponent>(ObjectComponent);
		const uint64 Id = CallRef->GetUniqueWorldID();
		if(Id == 0)
		{
			NewObjects.Add(CallRef);
		}
		else
		{
			if(IDsInUse.Contains(Id))
			{
				//Happens when copying an already indexed object only keep one of them
				CallRef->SetUniqueWorldID(0, FSetUniqueWorldIdKey());
				UE_LOG(LogUniqueWorldId, Warning, TEXT("Multiple objects with the same index detected"));
				NewObjects.Add(CallRef);
			}
			else IDsInUse.Add(Id);
		}
	}
		
	uint64 LastID = 0;
	TArray<std::pair<uint64, uint64>> UnusedIndices;
	while(true)
	{
		const uint64 NextID = GetLowestValidID(IDsInUse);
		IDsInUse.Remove(NextID);
		if(NextID - LastID > 1)
		{
			UnusedIndices.Add(std::pair(LastID, NextID));
		}
		if(NextID == std::numeric_limits<uint64>::max()) break;
		LastID = NextID;
	}
	
	uint64 UsedRange = 0;
	std::pair<uint64, uint64> BoundariesToFillNow;
	if(bAllowReusingIndices) BoundariesToFillNow = UnusedIndices[0];
	else
	{
		//If we should not reuse indices we first try to fill the indices above the currently highest index
		BoundariesToFillNow = UnusedIndices.Last();
	}
	
	
	for(USavableObjectMarkerComponent* SavableObject : NewObjects)
	{
		if(BoundariesToFillNow.second - BoundariesToFillNow.first <= UsedRange)
		{
			UnusedIndices.Remove(BoundariesToFillNow);
			//Independent of bAllowReusingIndices we restart at the lowest boundaries after filling the first boundaries
			BoundariesToFillNow = UnusedIndices[0];
		}
		if(UnusedIndices.IsEmpty()) { checkNoEntry(); }
		const uint64 TargetId = BoundariesToFillNow.first + UsedRange;
		SavableObject->SetUniqueWorldID(TargetId, FSetUniqueWorldIdKey());
		
		UE_LOG(LogUniqueWorldId, Log, TEXT("%s was set to index %s"), *SavableObject->GetOwner()->GetName(),
			*FString::FromInt(TargetId));
		UsedRange++;
	}
}

