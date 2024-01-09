// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HierarchicalInstancedStaticMeshObject.generated.h"

UCLASS()
class MAPROJECT_API AHierarchicalInstancedStaticMeshObject : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AHierarchicalInstancedStaticMeshObject();

protected:
	UPROPERTY(EditAnywhere)
	UHierarchicalInstancedStaticMeshComponent* HierarchicalInstancedStaticMesh;
	
#if WITH_EDITORONLY_DATA
	UFUNCTION(CallInEditor, Category = Debugging)
	void ReplaceAllRelatedStaticMeshes() const;
#endif
};
