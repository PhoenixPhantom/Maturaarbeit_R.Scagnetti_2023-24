// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/HierarchicalInstancedStaticMeshObject.h"


// Sets default values
AHierarchicalInstancedStaticMeshObject::AHierarchicalInstancedStaticMeshObject()
{
	PrimaryActorTick.bCanEverTick = false;
	HierarchicalInstancedStaticMesh = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("HISM"));
}

#if WITH_EDITORONLY_DATA
void AHierarchicalInstancedStaticMeshObject::ReplaceAllRelatedStaticMeshes() const
{
	TArray<AActor*> StaticMeshActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(),  StaticMeshActors);
	TArray<AStaticMeshActor*> StaticMeshesToRemove;
	for(AActor* Actor : StaticMeshActors)
	{
		AStaticMeshActor* StaticMeshActor = CastChecked<AStaticMeshActor>(Actor);
		if(StaticMeshActor->GetStaticMeshComponent()->GetStaticMesh() != HierarchicalInstancedStaticMesh->GetStaticMesh()) continue;
		HierarchicalInstancedStaticMesh->AddInstance(StaticMeshActor->GetActorTransform(), true);
		StaticMeshesToRemove.Add(StaticMeshActor);
	}

	for(AStaticMeshActor* StaticMeshActor : StaticMeshesToRemove)
	{
		StaticMeshActor->Destroy();
	}
}
#endif

