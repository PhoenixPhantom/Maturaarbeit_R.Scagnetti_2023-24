// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/Navigation/PatrolManagerComponent.h"

#include "Utility/Navigation/PatrolPath.h"


// Sets default values for this component's properties
UPatrolManagerComponent::UPatrolManagerComponent(): NextIndex(-1),
	bIncreasingIndex(false), PatrolPath(nullptr)
{
	PrimaryComponentTick.bCanEverTick = false;
}

FVector UPatrolManagerComponent::GetNextPathPointLocation(bool StartFromClosest)
{
	if(!IsValid(PatrolPath)){ return FVector(NAN);	}
	if(StartFromClosest || NextIndex < 0) GetPreferredNextPoint();
	return PatrolPath->GetAbsolutePointLocation(NextIndex);
}

int32 UPatrolManagerComponent::AdvanceToNextPathPoint()
{
	if(bIncreasingIndex)
	{
		//move through the list in forward direction


		if(NextIndex + 1 >= PatrolPath->GetNumPathPoints())
		{
			if(PatrolPath->IsLoopingPathPoints()) NextIndex = 0;
			else
			{
				//Return to the point before the current one and reverse the direction
				NextIndex--;
				bIncreasingIndex = false;
			}
		}
		else NextIndex++; //Go to the next point in order
	}
	else
	{
		//move through the list in backward direction

		
		if(NextIndex <= 0)
		{
			if(PatrolPath->IsLoopingPathPoints()) NextIndex = PatrolPath->GetNumPathPoints() - 1;
			else
			{
				//Return to the point before the current one and reverse the direction
				NextIndex++;
				bIncreasingIndex = true;
			}
		}
		else NextIndex--; //Go to the next point in order
	}
	
	return NextIndex;
}

void UPatrolManagerComponent::GetPreferredNextPoint()
{
	const FVector& OwnerLocation = GetOwner()->GetActorLocation();

	float ShortestDistance = std::numeric_limits<float>::max();
	int32 ShortestDistanceIndex = -1;
	for(int32 i = 0; i < PatrolPath->GetNumPathPoints(); i++)
	{
		const float Distance = FVector::Distance(OwnerLocation, PatrolPath->GetAbsolutePointLocation(i));
		if(Distance > ShortestDistance) continue;
		ShortestDistance = Distance;
		ShortestDistanceIndex = i;
	}

	if(ShortestDistanceIndex < 0) return;
	NextIndex = ShortestDistanceIndex;
}

